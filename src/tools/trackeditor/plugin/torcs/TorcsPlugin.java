/*
 *   TorcsPlugin.java
 *   Created on 9 ��� 2005
 *
 *    The TorcsPlugin.java is part of TrackEditor-0.6.0.
 *
 *    TrackEditor-0.6.0 is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    TrackEditor-0.6.0 is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with TrackEditor-0.6.0; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
package plugin.torcs;

import gui.EditorFrame;

import java.awt.event.ActionEvent;
import java.io.File;
import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.ImageIcon;
import javax.swing.JFileChooser;
import javax.swing.JMenuItem;

import plugin.Plugin;
import utils.CustomFileFilter;
import utils.Editor;

/**
 * @author Charalampos Alexopoulos
 * 
 * TODO To change the template for this generated type comment go to Window -
 * Preferences - Java - Code Style - Code Templates
 */
public class TorcsPlugin implements Plugin
{
	//private Properties		properties	= Properties.getInstance();
	protected EditorFrame	editor;
	private ImportAction	importAction;
	private ExportAction	exportAction;
	private JMenuItem		importMenuItem;
	private JMenuItem		exportMenuItem;

	private String sep = System.getProperty("file.separator");

	/**
	 *  
	 */
	public TorcsPlugin(EditorFrame editor)
	{
		this.editor = editor;
		importAction = new ImportAction("Speed Dreams", null, "Speed Dreams xml file", null);
		exportAction = new ExportAction("Speed Dreams", null, "Speed Dreams xml file", null);
	}

	public void importTrack()
	{
		String tmp = "";
		JFileChooser fc = new JFileChooser();
		Action folder = fc.getActionMap().get("New Folder");
		folder.setEnabled(false);
		fc.setSelectedFiles(null);
		fc.setSelectedFile(null);
		fc.rescanCurrentDirectory();
		fc.setApproveButtonMnemonic(0);
		fc.setDialogTitle("Import track from Xml");
		fc.setVisible(true);
		fc.setCurrentDirectory(new File(System.getProperty("user.dir") + "/tracks"));
		//fc.setCurrentDirectory(new File("/usr/share/games/torcs/tracks"));
		CustomFileFilter filter = new CustomFileFilter();
		
		filter.addValid(".xml");
		filter.addInvalid(".prj.xml");
		filter.setDescription("*.xml");
		fc.setFileFilter(filter);
		int result = fc.showOpenDialog(editor);
		if (result == JFileChooser.APPROVE_OPTION)
		{
			tmp = fc.getSelectedFile().toString();
			Editor.getProperties().getHeader().setName(tmp.substring(tmp.lastIndexOf(sep) + 1, tmp.lastIndexOf(".")));
			tmp = tmp.substring(0, tmp.lastIndexOf(sep));
			Editor.getProperties().setPath(tmp);
			tmp = Editor.getProperties().getPath().substring(0,tmp.lastIndexOf(sep));
			tmp = tmp.substring(tmp.lastIndexOf(sep)+1,tmp.length());
			Editor.getProperties().getHeader().setCategory(tmp.substring(tmp.lastIndexOf(sep) + 1));
			File file;
			file = new File(Editor.getProperties().getPath() + sep + Editor.getProperties().getHeader().getName() + ".xml");
			readFile(file);
			
		}
	}
	
	public void readFile(File file)
	{
		try
		{
			XmlReader.readXml(file.getAbsolutePath());
		} catch (Exception e)
		{
			//				message(e.getMessage(), "The file " + file.getAbsolutePath()
			// + " is not valid");
			e.printStackTrace();
		}
		editor.refresh();
	}

	public void exportTrack()
	{
		XmlWriter.writeXml();
	}
	/**
	 * This method initializes importMenuItem
	 * 
	 * @return javax.swing.JMenuItem
	 */
	public JMenuItem getImportMenuItem()
	{
		if (importMenuItem == null)
		{
			importMenuItem = new JMenuItem();
			importMenuItem.setAction(importAction);
			importMenuItem.setIcon(null);
		}
		return importMenuItem;
	}
	/**
	 * This method initializes exportMenuItem
	 * 
	 * @return javax.swing.JMenuItem
	 */
	public JMenuItem getExportMenuItem()
	{
		if (exportMenuItem == null)
		{
			exportMenuItem = new JMenuItem();
			exportMenuItem.setAction(exportAction);
			exportMenuItem.setIcon(null);
		}
		return exportMenuItem;
	}

	public class ImportAction extends AbstractAction
	{
		public ImportAction(String text, ImageIcon icon, String desc, Integer mnemonic)
		{
			super(text, icon);
			putValue(SHORT_DESCRIPTION, desc);
			putValue(MNEMONIC_KEY, mnemonic);
		}
		public void actionPerformed(ActionEvent e)
		{
			importTrack();
		}
	}

	private class ExportAction extends AbstractAction
	{
		public ExportAction(String text, ImageIcon icon, String desc, Integer mnemonic)
		{
			super(text, icon);
			putValue(SHORT_DESCRIPTION, desc);
			putValue(MNEMONIC_KEY, mnemonic);
		}
		public void actionPerformed(ActionEvent e)
		{
			System.out.println("Call exportXml");
			exportTrack();
		}
	}

}
