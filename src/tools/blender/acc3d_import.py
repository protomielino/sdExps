#!BPY

""" Registration info for Blender menus:
Name: 'Speed Dreams / Torcs (.acc)...'
Blender: 248
Group: 'Import'
Tip: 'Import a Speed Dreams / Torcs (.acc) file.'
"""

__author__ = "Brian Gavin based on Willian P. Germano .ac Importer"
__url__ = ("blender", "elysiun", "Speed Dreams's homepage, http://http://speed-dreams.sourceforge.net",
	"PLib 3d gaming lib, http://plib.sf.net")
__version__ = "2.43 2007-02-12"

__bpydoc__ = """\
This script imports Speed Dreams / Torcs AC3D models into Blender.

AC3D is a simple and affordable commercial 3d modeller also built with OpenGL.
The .ac file format is an easy to parse text format well supported,
for example, by the PLib 3d gaming library.

Supported:<br>
    UV-textured meshes with hierarchy (grouping) information.

Missing:<br>
    The url tag is irrelevant for Blender.

Known issues:<br>
    - Some objects may be imported with wrong normals due to wrong information in the model itself. This can be noticed by strange shading, like darker than expected parts in the model. To fix this, select the mesh with wrong normals, enter edit mode and tell Blender to recalculate the normals, either to make them point outside (the usual case) or inside.<br>

Config Options:<br>
    - textures dir (string): if non blank, when imported texture paths are
wrong in the .ac file, Blender will also look for them at this dir.

Notes:<br>
   - When looking for assigned textures, Blender tries in order: the actual
paths from the .ac file, the .ac file's dir and the default textures dir path
users can configure (see config options above).
"""

#
# --------------------------------------------------------------------------
# AC3DImport version 2.43 Feb 12, 2007
# Program versions: Blender 2.43 and AC3Db files (means version 0xb)
# changed: updated for new Blender version, Mesh module
# --------------------------------------------------------------------------
# Thanks: Melchior Franz for extensive bug testing and reporting, making this
# version cope much better with old or bad .ac files, among other improvements;
# Stewart Andreason for reporting a serious crash.
# --------------------------------------------------------------------------
# ***** BEGIN GPL LICENSE BLOCK *****
#
# Copyright (C) 2004-2007: Willian P. Germano, wgermano _at_ ig.com.br
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# ***** END GPL LICENCE BLOCK *****
# --------------------------------------------------------------------------

# Note:
# Blender doesn't handle n-gons (polygons with more than 4 vertices):
#  The script triangulates them, but concave polygons come out wrong and need
#  fixing. Avoiding or triangulating concave n-gons in AC3D is a simple way to
#  avoid problems.

from math import radians

import Blender
from Blender import Scene, Object, Mesh, Lamp, Registry, sys as bsys, Window, Image, Material, Texture
from Blender.sys import dirsep
from Blender.Mathutils import Vector, Matrix, Euler

# Default folder for AC3D textures, to override wrong paths, change to your
# liking or leave as "":
TEXTURES_DIR = "c:\\torcs-ng-sdl\\share\\data\\textures\\"
TEXTURES_DIR2 = "c:\\torcs-ng-sdl\\share\\data\\textures\\"
tooltips = {
	'TEXTURES_DIR': 'additional dir to look for missing textures'
}

def update_registry():
	global TEXTURES_DIR
	rd = dict([('TEXTURES_DIR', TEXTURES_DIR)])
	Registry.SetKey('acc3d_import', rd, True)

rd = Registry.GetKey('acc3d_import', True)

if rd:
	if 'GROUP' in rd:
		update_registry()
	TEXTURES_DIR = rd['TEXTURES_DIR']
else: update_registry()

if TEXTURES_DIR:
	oldtexdir = TEXTURES_DIR
	if dirsep == '/': TEXTURES_DIR = TEXTURES_DIR.replace('\\', '/')
	if TEXTURES_DIR[-1] != dirsep: TEXTURES_DIR = "%s%s" % (TEXTURES_DIR, dirsep)
	if oldtexdir != TEXTURES_DIR: update_registry()


VERBOSE = True
rd = Registry.GetKey('General', True)
if rd:
	if rd.has_key('verbose'):
		VERBOSE = rd['verbose']


errmsg = ""

# Matrix to align ac3d's coordinate system with Blender's one,
# it's a -90 degrees rotation around the x axis:
AC_TO_BLEND_MATRIX = Matrix([1, 0, 0], [0, 0, 1], [0, -1, 0])

AC_WORLD = 0
AC_GROUP = 1
AC_POLY = 2
AC_LIGHT = 3
AC_OB_TYPES = {
	'world': AC_WORLD,
	'group': AC_GROUP,
	'poly':  AC_POLY,
	'light':  AC_LIGHT
	}

def inform(msg):
	global VERBOSE
	if VERBOSE: print msg

def euler_in_radians(eul):
	"Used while there's a bug in the BPY API"
	eul.x = radians(eul.x)
	eul.y = radians(eul.y)
	eul.z = radians(eul.z)
	return eul

class Obj:

	def __init__(self, type):
		self.type = type
		self.dad = None
		self.name = ''
		self.data = ''
		self.tex = ''
		self.tex2 = ''
		self.texrep = [1,1]
		self.texoff = None
		self.loc = []
		self.rot = []
		self.size = []
		self.crease = 30
		self.vlist = []
		self.flist_cfg = []
		self.flist_v = []
		self.flist_uv = []
		self.flist_uv2 = []
		self.elist = []
		self.matlist = []
		self.kids = 0

		self.bl_obj = None # the actual Blender object created from this data

class AC3DImport:

	def __init__(self, filename):

		inform('Speed Dreams / Torcs ACC file importer version 1.0')

		global errmsg

		self.scene = Scene.GetCurrent()

		self.i = 0
		errmsg = ''
		self.importdir = bsys.dirname(filename)

		try:
			file = open(filename, 'r')
		except IOError, (errno, strerror):
			errmsg = "IOError #%s: %s" % (errno, strerror)
			Blender.Draw.PupMenu('ERROR: %s' % errmsg)
			inform(errmsg)
			return None
		header = file.read(5)
		header, version = header[:4], header[-1]
		if header != 'AC3D':
			file.close()
			errmsg = 'AC3D header not found (invalid file)'
			Blender.Draw.PupMenu('ERROR: %s' % errmsg)
			inform(errmsg)
			return None
		elif version != 'b':
			inform('AC3D file version 0x%s.' % version)
			inform('This importer is for version 0xb, so it may fail.')

		self.token = {'OBJECT':		self.parse_obj,
					  'numvert':	self.parse_vert,
					  'numsurf':	self.parse_surf,
					  'name':		self.parse_name,
					  'data':		self.parse_data,
					  'kids':		self.parse_kids,
					  'loc':		self.parse_loc,
					  'rot':		self.parse_rot,
					  'MATERIAL':	self.parse_mat,
					  'texture':	self.parse_tex,
					  'texrep':		self.parse_texrep,
					  'texoff':		self.parse_texoff,
					  'crease':		self.parse_crease}

		self.objlist = []
		self.mlist = []
		self.kidsnumlist = []
		self.dad = None

		self.lines = file.readlines()
		self.lines.append('')
		self.parse_file()
		file.close()

		self.testAC3DImport()

	def parse_obj(self, value):
		kidsnumlist = self.kidsnumlist
		if kidsnumlist:
			while not kidsnumlist[-1]:
				kidsnumlist.pop()
				if kidsnumlist:
					self.dad = self.dad.dad
				else:
					inform('Ignoring unexpected data at end of file.')
					return -1 # bad file with more objects than reported
			kidsnumlist[-1] -= 1
		new = Obj(AC_OB_TYPES[value])
		new.dad = self.dad
		new.name = value
		self.objlist.append(new)

	def parse_kids(self, value):
		kids = int(value)
		if kids:
			self.kidsnumlist.append(kids)
			self.dad = self.objlist[-1]
		self.objlist[-1].kids = kids

	def parse_name(self, value):
		name = value.split('"')[1]
		self.objlist[-1].name = name

	def parse_data(self, value):
		data = self.lines[self.i].strip()
		self.objlist[-1].data = data

	def parse_tex(self, value):
		line = self.lines[self.i - 1] # parse again to properly get paths with spaces
		texture = line.split(' ')[1]
		if texture!='empty_texture_no_mapping':
			if self.objlist[-1].tex == '':
				texture = texture.split('"')[1]
				self.objlist[-1].tex = texture
			else:
				texture2 = texture.split('"')[1]
				self.objlist[-1].tex2 = texture2



	def parse_texrep(self, trash):
		trep = self.lines[self.i - 1]
		trep = trep.split()
		trep = [float(trep[1]), float(trep[2])]
		self.objlist[-1].texrep = trep
		self.objlist[-1].texoff = [0, 0]

	def parse_texoff(self, trash):
		toff = self.lines[self.i - 1]
		toff = toff.split()
		toff = [float(toff[1]), float(toff[2])]
		self.objlist[-1].texoff = toff

	def parse_mat(self, value):
		i = self.i - 1
		lines = self.lines
		line = lines[i].split()
		mat_name = 'empty'
		mat_col = mat_amb = mat_emit = mat_spec_col = [0,0,0]
		mat_alpha = 1
		mat_spec = 1.0

		while line[0] == 'MATERIAL':
			mat_name = line[1].split('"')[1]
			mat_col = map(float,[line[3],line[4],line[5]])
			v = map(float,[line[7],line[8],line[9]])
			mat_amb = (v[0]+v[1]+v[2]) / 3.0
			v = map(float,[line[11],line[12],line[13]])
			mat_emit = (v[0]+v[1]+v[2]) / 3.0
			mat_spec_col = map(float,[line[15],line[16],line[17]])
			mat_spec = float(line[19]) / 64.0
			mat_alpha = float(line[-1])
			mat_alpha = 1 - mat_alpha
			self.mlist.append([mat_name, mat_col, mat_amb, mat_emit, mat_spec_col, mat_spec, mat_alpha])
			i += 1
			line = lines[i].split()

		self.i = i

	def parse_rot(self, trash):
		i = self.i - 1
		ob = self.objlist[-1]
		rot = self.lines[i].split(' ', 1)[1]
		rot = map(float, rot.split())
		matrix = Matrix(rot[:3], rot[3:6], rot[6:])
		ob.rot = matrix
		size = matrix.scalePart() # vector
		ob.size = size

	def parse_loc(self, trash):
		i = self.i - 1
		loc = self.lines[i].split(' ', 1)[1]
		loc = map(float, loc.split())
		self.objlist[-1].loc = Vector(loc)

	def parse_crease(self, value):
		# AC3D: range is [0.0, 180.0]; Blender: [1, 80]
		value = float(value)
		self.objlist[-1].crease = int(value)

	def parse_vert(self, value):
		i = self.i
		lines = self.lines
		obj = self.objlist[-1]
		vlist = obj.vlist
		n = int(value)

		while n:
			linet = lines[i].split()
			line = map(float, linet)
			#Remove the normal values
			if len(line)>5:
				del line[5]
			if len(line)>4:
				del line[4]
			if len(line)>3:
				del line[3]

			vlist.append(line)
			n -= 1
			i += 1

		if vlist: # prepend a vertex at 1st position to deal with vindex 0 issues
			vlist.insert(0, line)

		self.i = i

	def parse_surf(self, value):
		i = self.i
		is_smooth = 0
		double_sided = 0
		lines = self.lines
		obj = self.objlist[-1]
		matlist = obj.matlist
		numsurf = int(value)
		NUMSURF = numsurf

		badface_notpoly = badface_multirefs = 0

		while numsurf:
			flags = lines[i].split()[1][2:]
			flag = lines[i].split()[1]
			tristrip = False
			trifan = False

			if ((flag=='0x14')|(flag=='0x24')|(flag=='0x34')):
				tristrip = True

			if flag =='0x30':
				trifan = True

			if len(flags) > 1:
				flaghigh = int(flags[0])
				flaglow = int(flags[1])
			else:
				flaghigh = 0
				flaglow = int(flags[0])

			is_smooth = flaghigh & 1
			twoside = flaghigh & 2
			mat = lines[i+1].split()
			mat = int(mat[1])
			if not mat in matlist: matlist.append(mat)
			refs = lines[i+2].split()
			refs = int(refs[1])
			i += 3
			face = []
			faces = []
			edges = []
			fuv = []
			fuv2 = []
			fuvs = []
			fuvs2 = []
			rfs = refs

			while rfs:
				line = lines[i].split()
				v = int(line[0]) + 1 # + 1 to avoid vindex == 0
				uv = [float(line[1]), float(line[2])]

				if len(line)>4:
					uv2 = [float(line[3]), float(line[4])]
					fuv2.append(Vector(uv2))

				face.append(v)
				fuv.append(Vector(uv))

				rfs -= 1
				i += 1



			#handle triangle strip convert into triangles
			if tristrip==True:
				index = 0
				even = True
				tricount = refs-2;
				while index<tricount:
					facet = []
					fuvt = []
					fuvt2 = []
					if even:
						facet.append(face[index])
						uv = fuv[index];
						fuvt.append(uv)
						if len(fuv2)>0:
							uv2 = fuv2[index];
							fuvt2.append(uv2)

						facet.append(face[index+1])
						uv = fuv[index+1];
						fuvt.append(uv)
						if len(fuv2)>0:
							uv2 = fuv2[index+1];
							fuvt2.append(uv2)

						facet.append(face[index+2])
						uv = fuv[index+2];
						fuvt.append(uv)
						if len(fuv2)>0:
							uv2 = fuv2[index+2];
							fuvt2.append(uv2)

						even = False
					else:
						facet.append(face[index])
						uv = fuv[index];
						fuvt.append(uv)
						if len(fuv2)>0:
							uv2 = fuv2[index];
							fuvt2.append(uv2)

						facet.append(face[index+2])

						uv = fuv[index+2];

						fuvt.append(uv)
						if len(fuv2)>0:
							uv2 = fuv2[index+2];
							fuvt2.append(uv2)

						facet.append(face[index+1])
						uv = fuv[index+1];
						fuvt.append(uv)
						if len(fuv2)>0:
							uv2 = fuv2[index+1];
							fuvt2.append(uv2)

						even = True


					index+=1
					#Don't add degenerate triangles this messes up Blender
					if facet[0]!=facet[1] and facet[0]!=facet[2]:
						faces.append(facet)
						fuvs.append(fuvt)
						fuvs2.append(fuvt2)

					elif trifan==True:
						index = 1
						while index<refs-2:
							facet = []
							fuvt = []
							fuvt2 = []
							facet.append(face[0])
							uv = fuv[0];
							fuvt.append(uv)
							if len(fuv2)>0:
								uv2 = fuv2[0];
								fuvt2.append(uv2)

							facet.append(face[index])
							uv = fuv[index];
							fuvt.append(uv)
							if len(fuv2)>0:
								uv2 = fuv2[index];
								fuvt2.append(uv2)

							facet.append(face[index+1])
							uv = fuv[index+1];
							fuvt.append(uv)
							if len(fuv2)>0:
								uv2 = fuv2[index+1];
								fuvt2.append(uv2)

						index+=2

			elif flaglow: # it's a line or closed line, not a polygon
				while len(face) >= 2:
					cut = face[:2]
					edges.append(cut)
					face = face[1:]

				if flaglow == 1 and edges: # closed line
					face = [edges[-1][-1], edges[0][0]]
					edges.append(face)


			else: # polygon

				Blender.Draw.PupMenu('Polygon found')
				# check for bad face, that references same vertex more than once
				lenface = len(face)
				if lenface < 3:
					# less than 3 vertices, not a face
					badface_notpoly += 1
				elif sum(map(face.count, face)) != lenface:
					# multiple references to the same vertex
					badface_multirefs += 1
				else: # ok, seems fine
					while len(face) > 4:
						cut = face[:4]
						cutuv = fuv[:4]
						face = face[3:]
						fuv = fuv[3:]
						face.insert(0, cut[0])
						fuv.insert(0, cutuv[0])
						faces.append(cut)
						fuvs.append(cutuv)

					faces.append(face)
					fuvs.append(fuv)

			obj.flist_cfg.extend([[mat, is_smooth, twoside]] * len(faces))
			obj.flist_v.extend(faces)
			obj.flist_uv.extend(fuvs)
			obj.flist_uv2.extend(fuvs2)
			obj.elist.extend(edges) # loose edges

			numsurf -= 1

		if badface_notpoly or badface_multirefs:
			inform('Object "%s" - ignoring bad faces:' % obj.name)
			if badface_notpoly:
				inform('\t%d face(s) with less than 3 vertices.' % badface_notpoly)
			if badface_multirefs:
				inform('\t%d face(s) with multiple references to a same vertex.' % badface_multirefs)

		self.i = i

	def parse_file(self):
		i = 1
		lines = self.lines
		line = lines[i].split()

		while line:
			kw = ''
			for k in self.token.keys():
				if line[0] == k:
					kw = k
					break
			i += 1
			if kw:
				self.i = i
				result = self.token[kw](line[1])
				if result:
					break # bad .ac file, stop parsing
				i = self.i
			line = lines[i].split()

	# for each group of meshes we try to find one that can be used as
	# parent of the group in Blender.
	# If not found, we can use an Empty as parent.
	def found_parent(self, groupname, olist):
		l = [o for o in olist if o.type == AC_POLY \
				and not o.kids and not o.rot and not o.loc]
		if l:
			for o in l:
				if o.name == groupname:
					return o
				#return l[0]
		return None

	def build_hierarchy(self):
		blmatrix = AC_TO_BLEND_MATRIX

		olist = self.objlist[1:]
		olist.reverse()

		scene = self.scene

		newlist = []

		for o in olist:
			kids = o.kids
			if kids:
				children = newlist[-kids:]
				newlist = newlist[:-kids]
				if o.type == AC_GROUP:
					parent = self.found_parent(o.name, children)
					if parent:
						children.remove(parent)
						o.bl_obj = parent.bl_obj
					else: # not found, use an empty
						empty = scene.objects.new('Empty', o.name)
						o.bl_obj = empty

				bl_children = [c.bl_obj for c in children if c.bl_obj != None]

				o.bl_obj.makeParent(bl_children, 0, 1)
				for child in children:
					blob = child.bl_obj
					if not blob: continue
					if child.rot:
						eul = euler_in_radians(child.rot.toEuler())
						blob.setEuler(eul)
					if child.size:
						blob.size = child.size
					if not child.loc:
						child.loc = Vector(0.0, 0.0, 0.0)
					blob.setLocation(child.loc)

			newlist.append(o)

		for o in newlist: # newlist now only has objs w/o parents
			blob = o.bl_obj
			if not blob:
				continue
			if o.size:
				o.bl_obj.size = o.size
			if not o.rot:
				blob.setEuler([1.5707963267948966, 0, 0])
			else:
				matrix = o.rot * blmatrix
				eul = euler_in_radians(matrix.toEuler())
				blob.setEuler(eul)
			if o.loc:
				o.loc *= blmatrix
			else:
				o.loc = Vector(0.0, 0.0, 0.0)
			blob.setLocation(o.loc) # forces DAG update, so we do it even for 0, 0, 0

		# XXX important: until we fix the BPy API so it doesn't increase user count
		# when wrapping a Blender object, this piece of code is needed for proper
		# object (+ obdata) deletion in Blender:
		for o in self.objlist:
			if o.bl_obj:
				o.bl_obj = None

	def testAC3DImport(self):

		FACE_TWOSIDE = Mesh.FaceModes['TWOSIDE']
		FACE_TEX = Mesh.FaceModes['TILES']
		#FACE_TEX = Mesh.FaceModes['TEX']
		MESH_AUTOSMOOTH = Mesh.Modes['AUTOSMOOTH']

		MAT_MODE_ZTRANSP = Material.Modes['ZTRANSP']
		MAT_MODE_TRANSPSHADOW = Material.Modes['TRANSPSHADOW']

		scene = self.scene

		bl_images = {} # loaded texture images
		missing_textures = [] # textures we couldn't find
		bl_textures = {} # loaded textures

		objlist = self.objlist[1:] # skip 'world'

		#bmat = []
		bmat = {}
		m = None
		has_transp_mats = False
		for mat in self.mlist:
			name = mat[0]
#			if name=='':
			name = 'BaseMaterial'
			if m== None:
				m = Material.New(name)

			m.rgbCol = (mat[1][0], mat[1][1], mat[1][2])
			m.amb = mat[2]
			m.emit = mat[3]
			m.specCol = (mat[4][0], mat[4][1], mat[4][2])
			m.spec = mat[5]
			m.alpha = mat[6]
			#force it to zero because that is how Speed Dreams / Torcs work
			m.alpha = 0.0
			if m.alpha < 1.0:
				m.mode |= MAT_MODE_ZTRANSP
				has_transp_mats = True
#			bmat[name]=m

#		if has_transp_mats:
#			for mat in bmat:
#				mat.mode |= MAT_MODE_TRANSPSHADOW

		obj_idx = 0 # index of current obj in loop
		for obj in objlist:
			if obj.type == AC_GROUP:
				continue
			elif obj.type == AC_LIGHT:
				light = Lamp.New('Lamp')
				object = scene.objects.new(light, obj.name)
				#object.select(True)
				obj.bl_obj = object
				if obj.data:
					light.name = obj.data
				continue

			# type AC_POLY:

			# old .ac files used empty meshes as groups, convert to a real ac group
			if not obj.vlist and obj.kids:
				obj.type = AC_GROUP
				continue



			mesh = Mesh.New()
			object = scene.objects.new(mesh, obj.name)
			#object.select(True)
			obj.bl_obj = object
			if obj.data: mesh.name = obj.data
			mesh.degr = obj.crease # will auto clamp to [1, 80]

			if not obj.vlist: # no vertices? nothing more to do
				continue

			#Setup UV Layers
			mesh.addUVLayer("ImageUV")
			mesh.addUVLayer("ShadowUV")
			mesh.activeUVLayer ="ImageUV"

			mesh.verts.extend(obj.vlist)

			objmat_indices = []

			for e in obj.elist:
				mesh.edges.extend(e)

			if obj.flist_v:
				mesh.faces.extend(obj.flist_v)

				facesnum = len(mesh.faces)

				if facesnum == 0: # shouldn't happen, of course
					continue

				mesh.faceUV = True

				# checking if the .ac file had duplicate faces (Blender ignores them)
				if facesnum != len(obj.flist_v):
					# it has, ugh. Let's clean the uv list:
					lenfl = len(obj.flist_v)
					flist = obj.flist_v
					uvlist = obj.flist_uv
					uvlist2 = obj.flist_uv2
					cfglist = obj.flist_cfg
					for f in flist:
						f.sort()
					fi = lenfl
					while fi > 0: # remove data related to duplicates
						fi -= 1
						if flist[fi] in flist[:fi]:
							uvlist.pop(fi)
							cfglist.pop(fi)
							if len(uvlist2)>0:
								uvlist2.pop(fi)

				img = None
				img2 = None

				if obj.tex != '':
					if obj.tex in bl_images.keys():
						img = bl_images[obj.tex]
					elif obj.tex not in missing_textures:
						texfname = None
						objtex = obj.tex
						baseimgname = bsys.basename(objtex)
						if bsys.exists(objtex) == 1:
							texfname = objtex
						else:
							if baseimgname.find('\\') > 0:
								baseimgname = bsys.basename(objtex.replace('\\','/'))
							objtex = bsys.join(self.importdir, baseimgname)
							if bsys.exists(objtex) == 1:
								texfname = objtex
							else:
								objtex = bsys.join(TEXTURES_DIR2, baseimgname)
								if bsys.exists(objtex):
									texfname = objtex
								else:
                                                                        inform("Couldn't find texture in alt path %s" % TEXTURES_DIR2)
						if texfname:
							try:
								img = Image.Load(texfname)
								# Commented because it's unnecessary:
								#img.xrep = int(obj.texrep[0])
								#img.yrep = int(obj.texrep[1])
								if img:
									bl_images[obj.tex] = img
							except:
								inform("THROW:  Couldn't load texture: %s" % baseimgname)
						else:
							missing_textures.append(obj.tex)
							inform("Couldn't find texture: %s" % baseimgname)

				if obj.tex2 != '':
					if obj.tex2 in bl_images.keys():
						img2 = bl_images[obj.tex2]
					elif obj.tex2 not in missing_textures:
						texfname = None
						objtex2 = obj.tex2
						baseimgname = bsys.basename(objtex2)
						if bsys.exists(objtex2) == 1:
							texfname = objtex2
						else:
							if baseimgname.find('\\') > 0:
								baseimgname = bsys.basename(objtex2.replace('\\','/'))
							objtex2 = bsys.join(self.importdir, baseimgname)
							if bsys.exists(objtex) == 1:
								texfname = objtex2
							else:
								objtex2 = bsys.join(TEXTURES_DIR2, baseimgname)
								if bsys.exists(objtex2):
                                                                        texfname = objtex2
								else:
                                                                        inform("Couldn't find texture in alt path %s" % objtex2)

						if texfname:
							try:
								img2 = Image.Load(texfname)
								# Commented because it's unnecessary:
								#img.xrep = int(obj.texrep[0])
								#img.yrep = int(obj.texrep[1])
								if img2:
									bl_images[obj.tex2] = img2
							except:
								inform("THROW:  Couldn't load texture: %s" % baseimgname)
						else:
							missing_textures.append(obj.tex2)
							inform("Couldn't find texture: %s" % baseimgname)

				if obj.tex not in bmat.keys():
					if img:
						#Create a new material with a texture attached to it
						m1 = Material.New(obj.tex)
						m1.rgbCol = m.rgbCol
						m1.amb = m.amb
						m1.emit = m.emit
						m1.specCol = m.specCol
						m1.spec = m.spec
						m1.alpha = m.alpha
						texname = 'Tex%s' %obj.tex
						texname2 = 'Tex2%s' %obj.tex

						if img:
							iname = img.getName()
							if iname not in bl_textures.keys():
								basetex = Texture.New(iname)
								basetex.setType('Image')
								map1=Texture.MapTo.COL|Texture.MapTo.ALPHA
								basetex.image = img
								bl_textures[iname] = basetex

							basetex = bl_textures[iname]
							m1.setTexture(0,basetex,Texture.TexCo.UV,map1)

						if img2:
							iname2 = img2.getName()
							if iname2 not in bl_textures.keys():
								basetex2 = Texture.New(iname2)
								basetex2.setType('Image')
								map2=Texture.MapTo.COL
								basetex2.image = img2
								bl_textures[iname2] = basetex2
							else:
								map2=Texture.MapTo.COL

							basetex2 = bl_textures[iname2]
							m1.setTexture(1,basetex2,Texture.TexCo.UV,map2)

						if m1.alpha < 1.0:
							m1.mode |= MAT_MODE_ZTRANSP
							has_transp_mats = True

						mtextures = m1.getTextures()
						tunit = 0
						for mtex in mtextures:
							if not ( mtex == None ):
								if tunit == 1:
									mtex.uvlayer ="ShadowUV"
									mtex.colfac=0.3
									tunit = tunit+1
								if tunit ==0:
									mtex.uvlayer ="ImageUV"

									tunit = tunit+1

						bmat[obj.tex]=m1


				if obj.tex != '':
					if obj.tex in bl_images.keys():
						img = bl_images[obj.tex]

				#TODO attach correct materials to objects
				if obj.tex in bmat.keys():
					mat1 = bmat[obj.tex]
					if len(mesh.materials)==0:
						mesh.materials+= [mat1]
				else:
					inform('Material %s not found\n' % obj.tex)

				for i in range(facesnum):
					f = obj.flist_cfg[i]
					fmat = f[0]
					is_smooth = f[1]
					twoside = f[2]
					bface = mesh.faces[i]
					bface.smooth = is_smooth
					if twoside: bface.mode |= FACE_TWOSIDE
					if img:
						bface.mode |= FACE_TEX
						bface.image = img
					#TODO handle material properly
					#bface.mat = objmat_indices.index(fmat)
					bface.mat = 0
					fuv = obj.flist_uv[i]
					if len(obj.flist_uv2)>0:
						fuv2 = obj.flist_uv2[i]

					if obj.texoff:
						uoff = obj.texoff[0]
						voff = obj.texoff[1]
						urep = obj.texrep[0]
						vrep = obj.texrep[1]
						for uv in fuv:
							uv[0] *= urep
							uv[1] *= vrep
							uv[0] += uoff
							uv[1] += voff

						if len(fuv2)>0:
							for uv2 in fuv2:
								uv2[0] *= urep
								uv2[1] *= vrep
								uv2[0] += uoff
								uv2[1] += voff

					mesh.activeUVLayer ="ImageUV"
					mesh.faces[i].uv = fuv

					if len(fuv2)>0:
						mesh.activeUVLayer ="ShadowUV"
						mesh.faces[i].uv = fuv2
#						for uv2 in fuv2:
#							inform('UV2 coords %.5f %.5f\n' % (uv2[0],uv2[1]))

					mesh.activeUVLayer ="ImageUV"

				# finally, delete the 1st vertex we added to prevent vindices == 0
				mesh.verts.delete(0)

				mesh.calcNormals()

				mesh.mode = MESH_AUTOSMOOTH

			obj_idx += 1

		self.build_hierarchy()
		scene.update()

# End of class AC3DImport

def filesel_callback(filename):

	inform("\nTrying to import AC3D model(s) from:\n%s ..." % filename)
	Window.WaitCursor(1)
	starttime = bsys.time()
	test = AC3DImport(filename)
	Window.WaitCursor(0)
	endtime = bsys.time() - starttime
	inform('Done! Data imported in %.3f seconds.\n' % endtime)

Window.EditMode(0)

Window.FileSelector(filesel_callback, "Import Speed Dreams / Torcs ACC", "*.acc")
