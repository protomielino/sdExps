/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2024-2025 Xavier Del Campo Romero
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <tgfclient.h>
#include <tgf.hpp>
#include <portability.h>
#include <cars.h>
#include <tracks.h>
#include <racemanagers.h>
#include <drivers.h>
#include "assets.h"
#include "downloadsmenu.h"
#include "downloadservers.h"
#include "hash.h"
#include "infomenu.h"
#include "repomenu.h"
#include "sha256.h"
#include "unzip.h"
#include "writebuf.h"
#include "writefile.h"
#include <curl/curl.h>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <istream>
#include <memory>
#include <stdexcept>

static const size_t KB = 1024, MB = KB * KB;
enum {THUMBNAILS = 4};

DownloadsMenu::transfer::transfer(CURL *h, done f, sink *s) :
    h(h),
    f(f),
    s(s)
{
}

static void recompute(unsigned ms, void *args)
{
    static_cast<DownloadsMenu *>(args)->recompute(ms);
}

static void config(void *args)
{
    static_cast<DownloadsMenu *>(args)->config();
}

static void config_exit(const std::vector<std::string> &repos, void *args)
{
    static_cast<DownloadsMenu *>(args)->config_exit(repos);
}

static void deinit(void *args)
{
    delete static_cast<DownloadsMenu *>(args);
}

static void toggle(tCheckBoxInfo *info)
{
    static_cast<DownloadsMenu *>(info->userData)->toggle();
}

static int randname(std::string &name)
{
    static const size_t len = 32;

    for (size_t i = 0; i < len; i++)
    {
        unsigned char b;

        if (portability::rand(&b, sizeof b))
        {
            GfLogError("%s: portability::rand failed\n", __func__);
            return -1;
        }

        char hex[sizeof "00"];
        int n = snprintf(hex, sizeof hex, "%02hhx", b);

        if (n < 0 || n >= static_cast<int>(sizeof hex))
        {
            GfLogError("snprintf(3) failed with %d\n", n);
            return -1;
        }

        name += hex;
    }

    return 0;
}

static int tmppath(std::string &path)
{
    const char *dir = GfLocalDir();

    if (!dir)
    {
        GfLogError("unexpected null GfLocalDir\n");
        return -1;
    }

    const std::string tmpdir = std::string(dir) + "/tmp/";

    if (GfDirCreate(tmpdir.c_str()) != GF_DIR_CREATED)
    {
        GfLogError("Failed to create directory %s\n", tmpdir.c_str());
        return -1;
    }

    std::string r;

    if (randname(r))
    {
        GfLogError("Failed to generate random file name\n");
        return -1;
    }

    path = tmpdir + r;
    return 0;
}

void DownloadsMenu::config()
{
    new RepoMenu(hscr, ::recompute, ::config_exit, this);
}

void DownloadsMenu::config_exit(const std::vector<std::string> &repos)
{
    for (auto a : assets)
        delete a;

    assets.clear();

    if (downloadservers_set(repos))
        GfLogError("downloadservers_set failed\n");
    else if (fetch_assets())
        GfLogError("fetch_assets failed\n");
}

unsigned DownloadsMenu::visible_entries() const
{
    unsigned ret = 0;

    for (const auto e : entries)
        if (visible(e->a) && e->state != entry::init)
            ret++;

    return ret;
}

void DownloadsMenu::toggle()
{
    unsigned d = visible_entries();

    while (offset && offset >= d)
        offset -= THUMBNAILS;

    update_ui();
}

int DownloadsMenu::check(CURLcode result, CURL *h, std::string &error) const
{
    int ret = -1;
    long response;
    CURLcode code;
    char *url;

    if ((code = curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE,
        &response)) != CURLE_OK)
    {
        const char *e = curl_easy_strerror(code);

        error = "Failed to retrieve response code: ";
        error += e;
    }
    else if ((code = curl_easy_getinfo(h, CURLINFO_EFFECTIVE_URL, &url))
        != CURLE_OK)
    {
        const char *e = curl_easy_strerror(code);

        error = "Failed to retrieve effective URL: ";
        error += e;
    }
    else if (response != 200)
    {
        error = url;
        error += ": unexpected HTTP status ";
        error += std::to_string(response);
    }
    else if (result != CURLE_OK)
    {
        const char *e = curl_easy_strerror(result);

        error = "Fetch failed: ";
        error += e;
    }
    else
        ret = 0;

    if (ret)
        GfLogError("%s\n", error.c_str());

    return ret;
}

int DownloadsMenu::dispatch(const struct CURLMsg *m)
{
    for (auto t = transfers.begin(); t != transfers.end(); t++)
    {
        CURL *h = m->easy_handle;

        if (t->h == h)
        {
            std::string error;
            sink *s = t->s;
            CURLcode result = m->data.result;
            int ret = 0;

            s->flush();

            if (check(result, h, error)
                || (this->*t->f)(result, h, s, error))
            {
                GfuiLabelSetText(hscr, error_label, error.c_str());
                ret = -1;
            }

            transfers.erase(t);

            CURLMcode c = curl_multi_remove_handle(multi, h);

            if (c != CURLM_OK)
                GfLogError("curl_multi_remove_handle failed with %s\n",
                    curl_multi_strerror(c));

            curl_easy_cleanup(h);
            delete s;
            return ret;
        }
    }

    GfLogError("no suitable easy handle found\n");
    return -1;
}

void DownloadsMenu::recompute(unsigned ms)
{
    int running;
    double msd = ms;

    do
    {
        double now = GfTimeClock();
        CURLMcode code = curl_multi_perform(multi, &running);

        if (code != CURLM_OK)
        {
            GfLogError("curl_multi_perform: %s\n", curl_multi_strerror(code));
            return;
        }
        else if ((code = curl_multi_poll(multi, NULL, 0, msd, 0)) != CURLM_OK)
        {
            GfLogError("curl_multi_poll: %s\n", curl_multi_strerror(code));
            return;
        }

        double elapsed = (GfTimeClock() - now) * 1000.0;

        if (elapsed < msd)
            msd -= elapsed;
        else
            msd = 0;

        int pending = 0;

        do
        {
            const CURLMsg *m = curl_multi_info_read(multi, &pending);

            if (m)
            {
                if (m->msg != CURLMSG_DONE)
                    GfLogError("unexpected msg %d\n", m->msg);

                dispatch(m);
            }
        } while (pending);
    } while (running && (unsigned)msd);
}

static size_t on_write(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    sink *s = static_cast<sink *>(userdata);
    size_t total = size * nmemb;

    if (size != 0 && total / size != nmemb)
    {
        GfLogError("size calculation wrapped around\n");
        return !size;
    }
    else if (s->append(ptr, total))
    {
        GfLogError("append failed\n");
        return !size;
    }

    return total;
}

int DownloadsMenu::add(const char *url, transfer::done f, sink *s, long max)
{
    CURLcode code;
    CURL *h = curl_easy_init();

    if (!h)
    {
        GfLogError("curl_easy_init failed\n");
        goto failure;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_URL, url)) != CURLE_OK)
    {
        GfLogError("curl_easy_setopt url: %s\n", curl_easy_strerror(code));
        goto failure;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_PROTOCOLS,
        CURLPROTO_HTTP | CURLPROTO_HTTPS)) != CURLE_OK)
    {
        GfLogError("curl_easy_setopt protocols: %s\n",
            curl_easy_strerror(code));
        goto failure;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 0L))
        != CURLE_OK)
    {
        GfLogError("curl_easy_setopt follow location: %s\n",
            curl_easy_strerror(code));
        goto failure;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, ::on_write))
        != CURLE_OK)
    {
        GfLogError("curl_easy_setopt writefunction: %s\n",
            curl_easy_strerror(code));
        goto failure;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_MAXFILESIZE, max)))
    {
        GfLogError("curl_easy_setopt maxfilesize: %s\n",
            curl_easy_strerror(code));
        goto failure;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_FAILONERROR, 1L)))
    {
        GfLogError("curl_easy_setopt failonerror: %s\n",
            curl_easy_strerror(code));
        goto failure;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_WRITEDATA,
        static_cast<void *>(s))) != CURLE_OK)
    {
        GfLogError("curl_easy_setopt writedata: %s\n",
            curl_easy_strerror(code));
        goto failure;
    }
    else if (curl_multi_add_handle(multi, h) != CURLM_OK)
    {
        GfLogError("curl_multi_add_handle failed\n");
        return -1;
    }

    transfers.push_back(transfer(h, f, s));
    return 0;

failure:
    curl_easy_cleanup(h);
    return -1;
}

static int get_size(size_t n, std::string &out)
{
    static const char *const suffixes[] = {"B", "KiB", "MiB", "GiB", "TiB"};
    float sz;
    size_t i;

    for (sz = n, i = 0; (unsigned long long)sz / 1024; i++, sz /= 1024.0f)
        ;

    if (i >= sizeof suffixes / sizeof *suffixes)
    {
        GfLogError("%s: maximum suffix exceeded\n", __func__);
        return -1;
    }

    const char *suffix = suffixes[i];

    if (i)
    {
        char s[sizeof "18446744073709551615.0"];
        int res = snprintf(s, sizeof s, "%.1f", sz);

        if (res < 0 || (unsigned)res >= sizeof s)
        {
            GfLogError("Maximum size exceeded\n");
            return -1;
        }

        out = s;
    }
    else
        out = std::to_string(n);

    out += " ";
    out += suffix;
    return 0;
}

bool DownloadsMenu::visible(const Asset &a) const
{
    switch (a.type)
    {
        case Asset::car:
            return GfuiCheckboxIsChecked(hscr, cars_cb);

        case Asset::track:
            return GfuiCheckboxIsChecked(hscr, tracks_cb);

        case Asset::driver:
            return GfuiCheckboxIsChecked(hscr, drivers_cb);
    }

    return false;
}

void DownloadsMenu::append(thumbnail *t, entry *e)
{
    const Asset &a = e->a;

    std::string size;

    if (get_size(a.size, size))
    {
        GfLogError("Failed to get size string representation\n");
        return;
    }

    bargs.push_back(barg(t, e));
    t->set(e->thumbnail, a.name, size);
}

void DownloadsMenu::process(thumbnail *t, entry *e, bool &appended,
    unsigned &offset)
{
    bool enable = false, show_progress = false, show_delete = false;
    float progress = 0.0f;

    switch (e->state)
    {
        case entry::init:
            appended = false;
            return;

        case entry::download:
            enable = true;
            break;

        case entry::update:
            enable = true;
            show_delete = true;
            break;

        case entry::fetching:
            show_progress = true;
            progress = e->progress;
            break;

        case entry::done:
            show_delete = true;
            break;
    }

    if (offset++ < this->offset)
    {
        appended = false;
        return;
    }

    t->set(enable, show_progress, progress, show_delete);
    append(t, e);
    appended = true;
}

void DownloadsMenu::update_ui()
{
    for (auto t : thumbnails)
        t->clear();

    bargs.clear();

    auto t = thumbnails.begin();
    unsigned offset = 0;

    for (entry *e : entries)
    {
        const Asset &a = e->a;

        if (t == thumbnails.end())
            break;
        else if (visible(a))
        {
            bool appended;

            process(*t, e, appended, offset);

            if (appended)
                t++;
        }
    }
}

static std::unique_ptr<hash> alloc_hash(const std::string &type)
{
    if (type == "sha256")
        return std::unique_ptr<hash>(new sha256());

    return nullptr;
}

int DownloadsMenu::check_hash(const entry *e, const std::string &path,
    std::string &error) const
{
    const Asset &a = e->a;
    const std::string &exp = a.hash, &type = a.hashtype;
    std::unique_ptr<hash> h = alloc_hash(type);
    std::string hash;

    if (!h)
    {
        error = "Unsupported hash type ";
        error += type;
        GfLogError("%s\n", error.c_str());
        return -1;
    }
    else if (h->run(path, hash))
    {
        error = "Failed to calculate hash";
        GfLogError("%s\n", error.c_str());
        return -1;
    }
    else if (hash != exp)
    {
        error = "Hash mismatch";
        GfLogError("%s: %s, expected=%s, got=%s\n",
            error.c_str(), path.c_str(), exp.c_str(), hash.c_str());
        return -1;
    }

    return 0;
}

static int write_revision(const Asset &a, const std::string &path)
{
    std::ofstream f(path + "/.revision", std::ios::binary);

    f << std::to_string(a.revision) << std::endl;
    return 0;
}

int DownloadsMenu::extract(const entry *e, const std::string &src,
    std::string &error) const
{
    const Asset &a = e->a;
    std::string name;

    if (randname(name))
    {
        error = "Failed to generate random directory name";
        GfLogError("randname failed\n");
        return -1;
    }

    std::string base = a.basedir(), tmp = base + name + "/";
    unzip u(src, tmp, a.directory);
    std::string dst = base + a.dstdir(),
        tmpd = tmp + a.directory;
    int ret = -1;

    if (u.run())
    {
        const char exc[] = "Failed to extract file";

        error = exc;
        GfLogError("%s %s\n", exc, src.c_str());
        goto end;
    }
    else if (write_revision(a, tmpd))
    {
        error = "Failed to write revision";
        GfLogError("write_revision failed\n");
        goto end;
    }
    else if (portability::rmdir_r(dst.c_str()))
    {
        error = "Failed to remove directory";
        GfLogError("rmdir_r %s failed\n", dst.c_str());
        goto end;
    }
    else if (rename(tmpd.c_str(), dst.c_str()))
    {
        const char *e = strerror(errno);

        error = "Failed to rename directory: ";
        error += e;
        GfLogError("rename(3) %s -> %s: %s\n", tmpd.c_str(), dst.c_str(), e);
        goto end;
    }

    ret = 0;

end:
    if (portability::rmdir_r(tmp.c_str()))
    {
        error = "Failed to remove directory";
        GfLogError("rmdir_r %s failed\n", tmp.c_str());
        ret = -1;
    }

    return ret;
}

int DownloadsMenu::save(entry *e, const std::string &path,
    std::string &error) const
{
    const Asset &a = e->a;
    std::string dir = a.basedir() + a.path();

    if (check_hash(e, path, error)
        || GfDirCreate(dir.c_str()) != GF_DIR_CREATED
        || extract(e, path, error))
        goto failure;
    else
        e->state = entry::done;

    return 0;

failure:
    if (remove(e->data.c_str()))
    {
        std::string s = strerror(errno);

        error = "Failed to remove file: ";
        error += s;
        GfLogError("remove(3) %s: %s\n", e->data.c_str(),
            s.c_str());
    }

    e->state = entry::download;
    e->data.clear();
    return -1;
}

int DownloadsMenu::asset_fetched(CURLcode result, CURL *h, const sink *s,
    std::string &error)
{
    const writefile *w = static_cast<const writefile *>(s);

    for (auto p = pargs.begin(); p != pargs.end(); p++)
    {
        pressedargs *pa = static_cast<pressedargs *>(w->args);

        if (*p == *pa)
        {
            pargs.erase(p);
            break;
        }
    }

    int ret = 0;

    for (auto e : entries)
    {
        const std::string &path = w->path;

        if (e->data == path)
        {
            ret = save(e, path, error);
            break;
        }
    }

    update_ui();
    return ret;
}

int DownloadsMenu::thumbnail_fetched(CURLcode result, CURL *h, const sink *s,
    std::string &error)
{
    const writefile *w = static_cast<const writefile *>(s);

    for (auto e : entries)
        if (e->thumbnail == w->path)
        {
            bool update;

            if (e->a.needs_update(update))
                e->state = entry::download;
            else if (update)
                e->state = entry::update;
            else
                e->state = entry::done;

            break;
        }

    update_ui();
    return 0;
}

static int get_extension(const std::string &name, std::string &ext)
{
    size_t i = name.find_last_of('.');

    if (i == std::string::npos)
        return -1;

    ext = name.substr(i);
    return 0;
}

int DownloadsMenu::fetch_thumbnails(const std::vector<Asset> &assets)
{
    for (const auto &a : assets)
    {
        bool dup = false;

        for (auto e : entries)
            if (a == e->a)
            {
                dup = true;
                break;
            }

        if (dup)
            continue;

        std::string path, ext;

        if (tmppath(path))
        {
            GfLogError("Failed to create a temporary file name\n");
            return -1;
        }
        else if (get_extension(a.thumbnail, ext))
        {
            GfLogError("Failed to get file name extension from %s\n",
                path.c_str());
            return -1;
        }

        path += ext;

        static const size_t max = 1 * MB;
        writefile *w = new writefile(path.c_str(), max);

        if (add(a.thumbnail.c_str(), &DownloadsMenu::thumbnail_fetched, w, max))
        {
            GfLogError("add failed\n");
            delete w;
            return -1;
        }

        entries.push_back(new entry(a, path));
    }

    return 0;
}

int DownloadsMenu::assets_fetched(CURLcode result, CURL *h, const sink *s,
    std::string &error)
{
    const writebuf *w = static_cast<const writebuf *>(s);
    char *ct;
    CURLcode code = curl_easy_getinfo(h, CURLINFO_CONTENT_TYPE, &ct);
    static const char exp_ct[] = "application/json";

    if (code != CURLE_OK)
    {
        const char *e = curl_easy_strerror(code);

        GfLogError("curl_easy_getinfo: %s\n", e);
        error = e;
        return -1;
    }
    else if (!ct)
    {
        static const char e[] = "Missing Content-Type";

        GfLogError("%s\n", e);
        error = e;
        return -1;
    }
    else if (strcmp(ct, exp_ct))
    {
        error = "Expected Content-Type ";
        error += exp_ct;
        error += ", got ";
        error += ct;

        GfLogError("%s\n", error.c_str());
        return -1;
    }

    Assets *a = new Assets();

    if (a->parse(static_cast<char *>(w->data()), w->size()))
    {
        static const char e[] = "Failed to parse assets list";

        GfLogError("%s\n", e);
        error = e;
        delete a;
        return -1;
    }
    else if (fetch_thumbnails(a->get()))
    {
        static const char e[] = "Failed to fetch thumbnails";

        GfLogError("%s\n", e);
        error = e;
        delete a;
        return -1;
    }

    assets.push_back(a);
    return 0;
}

int DownloadsMenu::fetch_assets()
{
    const size_t max = 2 * MB;
    std::vector<std::string> urls;

    if (downloadservers_get(urls))
    {
        GfLogError("downloadservers_get failed\n");
        return -1;
    }

    for (const auto &u : urls)
    {
        writebuf *w = new writebuf(max);

        if (add(u.c_str(), &DownloadsMenu::assets_fetched, w, max))
        {
            GfLogError("add failed\n");
            delete w;
        }
    }

    return 0;
}

int DownloadsMenu::progress(const pressedargs *p, float pt) const
{
    thumbnail *t = p->t;
    entry *e = p->e;

    for (const auto &b : bargs)
        if (t == b.first && e == b.second)
        {
            e->progress = pt;
            t->progress(pt);
            break;
        }

    return 0;
}

static int progress(size_t n, size_t max, void *args)
{
    const struct DownloadsMenu::pressedargs *p =
        static_cast<const struct DownloadsMenu::pressedargs *>(args);
    DownloadsMenu *m = p->m;

    return m->progress(p, 100.0f * (float)n / (float)max);
}

void DownloadsMenu::pressed(thumbnail *t)
{
    for (const auto &b : bargs)
    {
        if (b.first != t)
            continue;

        entry *e = b.second;
        const Asset &a = e->a;
        std::string path;

        if (tmppath(path))
        {
            GfLogError("tmppath failed\n");
            return;
        }

        pargs.push_back(pressedargs(this, t, e));

        writefile *w = new writefile(path.c_str(), a.size, ::progress,
            &pargs.back());

        if (add(a.url.c_str(), &DownloadsMenu::asset_fetched, w, a.size))
        {
            GfLogError("add failed\n");
            delete w;
            return;
        }

        e->data = path;
        e->state = entry::fetching;
        update_ui();
        break;
    }
}

static void pressed(thumbnail *t, void *arg)
{
    DownloadsMenu *m = static_cast<DownloadsMenu *>(arg);

    m->pressed(t);
}

void DownloadsMenu::on_delete(thumbnail *t)
{
    for (const auto &b : bargs)
    {
        if (b.first != t)
            continue;

        entry *e = b.second;
        const Asset &a = e->a;
        std::string pathstr = a.basedir() + a.dstdir();
        const char *path = pathstr.c_str();

        if (portability::rmdir_r(path))
            GfLogError("rmdir_r %s failed\n", path);
        else
        {
            e->state = entry::download;
            update_ui();
        }

        break;
    }
}

void DownloadsMenu::on_info(thumbnail *t)
{
    for (const auto &b : bargs)
    {
        if (b.first != t)
            continue;

        new InfoMenu(hscr, ::recompute, this, b.second);
        break;
    }
}

static void on_delete(thumbnail *t, void *arg)
{
    DownloadsMenu *m = static_cast<DownloadsMenu *>(arg);

    m->on_delete(t);
}

static void on_info(thumbnail *t, void *arg)
{
    DownloadsMenu *m = static_cast<DownloadsMenu *>(arg);

    m->on_info(t);
}

void DownloadsMenu::prev_page()
{
    if (!offset)
        return;

    offset -= THUMBNAILS;
    update_ui();
}

void DownloadsMenu::next_page()
{
    unsigned d = visible_entries();

    if (THUMBNAILS > d || offset >= d - THUMBNAILS)
        return;

    offset += THUMBNAILS;
    update_ui();
}

static void prev_page(void *arg)
{
    DownloadsMenu *m = static_cast<DownloadsMenu *>(arg);

    m->prev_page();
}

static void next_page(void *arg)
{
    DownloadsMenu *m = static_cast<DownloadsMenu *>(arg);

    m->next_page();
}

DownloadsMenu::DownloadsMenu(void *prevMenu) :
    hscr(GfuiScreenCreate()),
    prev(prevMenu),
    multi(curl_multi_init()),
    offset(0)
{
    if (!hscr)
        throw std::runtime_error("GfuiScreenCreate failed");

    void *param = GfuiMenuLoad("downloadsmenu.xml");

    if (!param)
        throw std::runtime_error("GfuiMenuLoad failed");
    else if (!hscr)
        throw std::runtime_error("GfuiScreenCreate failed");
    else if (!multi)
        throw std::runtime_error("curl_multi_init failed");
    else if (!GfuiMenuCreateStaticControls(hscr, param))
        throw std::runtime_error("GfuiMenuCreateStaticControls failed");
    else if ((error_label =
        GfuiMenuCreateLabelControl(hscr, param, "error")) < 0)
        throw std::runtime_error("GfuiMenuCreateLabelControl error failed");
    else if ((cars_cb =
        GfuiMenuCreateCheckboxControl(hscr, param, "carscheckbox",
            this, ::toggle)) < 0)
        throw std::runtime_error("GfuiMenuCreateCheckboxControl cars failed");
    else if ((tracks_cb =
        GfuiMenuCreateCheckboxControl(hscr, param, "trackscheckbox",
            this, ::toggle)) < 0)
        throw std::runtime_error("GfuiMenuCreateCheckboxControl tracks failed");
    else if ((drivers_cb =
        GfuiMenuCreateCheckboxControl(hscr, param, "driverscheckbox",
            this, ::toggle)) < 0)
        throw std::runtime_error("GfuiMenuCreateCheckboxControl drivers failed");
    else if (GfuiMenuCreateButtonControl(hscr, param, "back", this,
        ::deinit) < 0)
        throw std::runtime_error("GfuiMenuCreateButtonControl back failed");
    else if (GfuiMenuCreateButtonControl(hscr, param, "config", this,
        ::config) < 0)
        throw std::runtime_error("GfuiMenuCreateButtonControl config failed");
    else if ((prev_arrow = GfuiMenuCreateButtonControl(hscr, param,
        "previous page arrow", this, ::prev_page)) < 0)
        throw std::runtime_error("GfuiMenuCreateButtonControl prev failed");
    else if ((next_arrow = GfuiMenuCreateButtonControl(hscr, param,
        "next page arrow", this, ::next_page)) < 0)
        throw std::runtime_error("GfuiMenuCreateButtonControl next failed");

    for (int i = 0; i < THUMBNAILS; i++)
    {
        std::string id = "thumbnail";

        id += std::to_string(i);
        thumbnails.push_back(new thumbnail(hscr, param, id, ::pressed,
            ::on_delete, ::on_info, this));
    }

    GfParmReleaseHandle(param);
    GfuiMenuDefaultKeysAdd(hscr);
    GfuiAddKey(hscr, GFUIK_ESCAPE, "Back to previous menu", this, ::deinit,
        NULL);
    GfuiScreenActivate(hscr);
    // This must be done after GfuiScreenActivate because this function
    // overwrites the recompute callback to a null pointer.
    GfuiApp().eventLoop().setRecomputeCB(::recompute, this);

    if (fetch_assets())
        throw std::runtime_error("fetch_assets failed");
}

DownloadsMenu::~DownloadsMenu()
{
    for (auto t : thumbnails)
        delete t;

    for (auto a : assets)
        delete a;

    for (auto e : entries)
        delete e;

    for (const auto &t : transfers)
    {
        curl_multi_remove_handle(multi, t.h);
        curl_easy_cleanup(t.h);
        delete t.s;
    }

    curl_multi_cleanup(multi);
    GfCars::reload();
    GfTracks::reload();
    GfRaceManagers::reload();
    GfDrivers::self()->reload();
    GfuiScreenRelease(hscr);
    GfuiScreenActivate(prev);
}
