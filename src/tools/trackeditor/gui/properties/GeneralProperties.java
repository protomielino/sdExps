/*
 *   GeneralProperties.java
 *   Created on 27 ??? 2005
 *
 *    The GeneralProperties.java is part of TrackEditor-0.3.1.
 *
 *    TrackEditor-0.3.1 is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    TrackEditor-0.3.1 is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with TrackEditor-0.3.1; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
package gui.properties;

import java.io.File;

import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JTextField;

import gui.EditorFrame;
import utils.Editor;

/**
 * @author babis
 *
 * TODO To change the template for this generated type comment go to Window -
 * Preferences - Java - Code Style - Code Templates
 */
public class GeneralProperties extends PropertyPanel
{
	private JLabel				nameLabel				= new JLabel();
	private JTextField			nameTextField			= new JTextField();
	private JLabel				categoryLabel			= new JLabel();
	private JComboBox<String>	categoryComboBox		= null;
	private JLabel				subcategoryLabel		= new JLabel();
	private JComboBox<String>	subcategoryComboBox		= null;
	private JLabel				versionLabel			= new JLabel();
	private JComboBox<String>	versionComboBox			= null;
	private JLabel				skyVersionLabel			= new JLabel();
	private JComboBox<String>	skyVersionComboBox		= null;
	private JLabel				pathLabel				= new JLabel();
	private JTextField			pathTextField			= new JTextField();
	private JButton				pathButton				= null;
	private JLabel				authorLabel				= new JLabel();
	private JTextField			authorTextField			= new JTextField();
	private JLabel				descriptionLabel		= new JLabel();
	private JTextField			descriptionTextField	= new JTextField();

	private final String sep = System.getProperty("file.separator");

	/**
	 *
	 */
	public GeneralProperties(EditorFrame frame)
	{
		super(frame);
		initialize();
	}

	/**
	 *
	 */
	private void initialize()
	{
		setLayout(null);
		setBorder(javax.swing.BorderFactory.createEtchedBorder(javax.swing.border.EtchedBorder.LOWERED));

		addLabel(this, 0, nameLabel, "Name", 110);
		addLabel(this, 1, categoryLabel, "Category", 110);
		addLabel(this, 2, subcategoryLabel, "Subcategory", 110);
		addLabel(this, 3, versionLabel, "Version", 110);
		addLabel(this, 4, skyVersionLabel, "Sky Version", 110);
		addLabel(this, 5, pathLabel, "Path", 80);
		addLabel(this, 6, authorLabel, "Author", 80);
		addLabel(this, 7, descriptionLabel, "Description", 80);

		addTextField(this, 0, nameTextField, Editor.getProperties().getHeader().getName(), 130, 150);

		add(getCategoryComboBox(), null);
		add(getSubcategoryComboBox(), null);
		add(getVersionComboBox(), null);
		add(getSkyVersionComboBox(), null);

		addTextField(this, 5, pathTextField, Editor.getProperties().getPath(), 80, 305);
		addTextField(this, 6, authorTextField, Editor.getProperties().getHeader().getAuthor(), 80, 390);
		addTextField(this, 7, descriptionTextField, Editor.getProperties().getHeader().getDescription(), 80, 390);

		add(getPathButton(), null);
	}

	/**
	 * This method initializes categoryComboBox
	 *
	 * @return javax.swing.JComboBox
	 */
	public JComboBox<String> getCategoryComboBox()
	{
		if (categoryComboBox == null)
		{
			String[] items = {"circuit", "development", "dirt", "gprix", "karting", "oval", "road", "speedway", "test"};
			categoryComboBox = new JComboBox<String>(items);
			categoryComboBox.setBounds(130, 35, 100, 20);
			categoryComboBox.setSelectedItem(Editor.getProperties().getHeader().getCategory());
		}
		return categoryComboBox;
	}

	/**
	 * This method initializes subcategoryComboBox
	 *
	 * @return javax.swing.JComboBox
	 */
	public JComboBox<String> getSubcategoryComboBox()
	{
		if (subcategoryComboBox == null)
		{
			String[] items = {"none", "short", "long"};
			subcategoryComboBox = new JComboBox<String>(items);
			subcategoryComboBox.setBounds(130, 60, 100, 20);
			String subcategory = Editor.getProperties().getHeader().getSubcategory();
			if (subcategory == null)
				subcategory = "none";
			subcategoryComboBox.setSelectedItem(subcategory);
		}
		return subcategoryComboBox;
	}

	/**
	 * This method initializes versionComboBox
	 *
	 * @return javax.swing.JComboBox
	 */
	public JComboBox<String> getVersionComboBox()
	{
		if (versionComboBox == null)
		{
			String[] items = {"3", "4", "5"};
			versionComboBox = new JComboBox<String>(items);
			versionComboBox.setBounds(130, 85, 100, 20);
			versionComboBox.setSelectedItem(Editor.getProperties().getHeader().getVersion() + "");
		}
		return versionComboBox;
	}

	/**
	 * This method initializes skyVersionComboBox
	 *
	 * @return javax.swing.JComboBox
	 */
	public JComboBox<String> getSkyVersionComboBox()
	{
		if (skyVersionComboBox == null)
		{
			String[] items = {"none", "1"};
			skyVersionComboBox = new JComboBox<String>(items);
			skyVersionComboBox.setBounds(130, 110, 100, 20);
			int version = Editor.getProperties().getHeader().getSkyVersion();
			String stringVersion;
			if (version == Integer.MAX_VALUE)
				stringVersion = "none";
			else
				stringVersion = version + "";
			skyVersionComboBox.setSelectedItem(stringVersion);
		}
		return skyVersionComboBox;
	}

	/**
	 * This method initializes pathButton
	 *
	 * @return javax.swing.JButton
	 */
	public JButton getPathButton()
	{
		if (pathButton == null)
		{
			pathButton = new JButton();
			pathButton.setBounds(390, 132, 80, 25);
			pathButton.setText("Browse");
			pathButton.addActionListener(new java.awt.event.ActionListener()
			{
				public void actionPerformed(java.awt.event.ActionEvent e)
				{
					selectPath();
				}
			});
		}
		return pathButton;
	}

	/**
	 *
	 */
	protected void selectPath()
	{
		JFileChooser fc = new JFileChooser();
		fc.setSelectedFiles(null);
		fc.setSelectedFile(null);
		fc.rescanCurrentDirectory();
		fc.setApproveButtonMnemonic(0);
		fc.setDialogTitle("Project path selection");
		fc.setVisible(true);
		fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
		fc.setCurrentDirectory(new File(System.getProperty("user.dir") + sep + "tracks"));
		int result = fc.showDialog(this, "Ok");
		if (result == JFileChooser.APPROVE_OPTION)
		{
			pathTextField.setText(fc.getSelectedFile().toString());
		}
	}

	public void exit()
	{
		MutableString stringResult = new MutableString();

		// the path is something/category/track
		String tmpPath = pathTextField.getText();
		String tmpName = nameTextField.getText();
		String tmpCategory = (String) getCategoryComboBox().getSelectedItem();

		if (isDifferent(tmpName, Editor.getProperties().getHeader().getName(), stringResult))
		{
			Editor.getProperties().getHeader().setName(stringResult.getValue());
			frame.documentIsModified = true;
		}

		if (isDifferent(tmpPath, Editor.getProperties().getPath(), stringResult))
		{
			Editor.getProperties().setPath(stringResult.getValue());
			frame.documentIsModified = true;
		}

		if (isDifferent(tmpCategory, Editor.getProperties().getHeader().getCategory(), stringResult))
		{
			Editor.getProperties().getHeader().setCategory(stringResult.getValue());
			frame.documentIsModified = true;
		}

		// get the track name from the path
		int index = tmpPath.lastIndexOf(sep);
		String nameFromPath = tmpPath.substring(index + 1);
		if (!nameFromPath.equals(tmpName))
		{
			// TODO Which one should we use?
			System.out.println("different!!! name from path: " + nameFromPath + " name: " + tmpName);
		}

		// remove the track name from the path
		String pathToCategory = tmpPath.substring(0, index);

		// get the category from the path
		index = pathToCategory.lastIndexOf(sep);
		String categoryFromPath = pathToCategory.substring(index + 1);
		if (!categoryFromPath.equals(tmpCategory))
		{
			// TODO  Which one should we use?
			System.out.println("category from path : " + categoryFromPath + " category : " + tmpCategory);
		}

		File path = new File(tmpPath);
		if (!path.exists())
		{
			path.mkdirs();
		}

		if (isDifferent((String) getSubcategoryComboBox().getSelectedItem(),
			Editor.getProperties().getHeader().getSubcategory(), stringResult))
		{
			Editor.getProperties().getHeader().setSubcategory(stringResult.getValue());
			frame.documentIsModified = true;
		}

		int version = Integer.parseInt((String) getVersionComboBox().getSelectedItem());
		if (version != Editor.getProperties().getHeader().getVersion())
		{
			Editor.getProperties().getHeader().setVersion(version);
			frame.documentIsModified = true;
		}

		String skyVersionString = (String) getSkyVersionComboBox().getSelectedItem();
		int skyVersion;
		if (skyVersionString == "none")
			skyVersion = Integer.MAX_VALUE;
		else
			skyVersion = Integer.parseInt(skyVersionString);
		if (skyVersion != Editor.getProperties().getHeader().getSkyVersion())
		{
			Editor.getProperties().getHeader().setSkyVersion(skyVersion);
			frame.documentIsModified = true;
		}

		if (isDifferent(authorTextField.getText(),
			Editor.getProperties().getHeader().getAuthor(), stringResult))
		{
			Editor.getProperties().getHeader().setAuthor(stringResult.getValue());
			frame.documentIsModified = true;
		}

		if (isDifferent(descriptionTextField.getText(),
			Editor.getProperties().getHeader().getDescription(), stringResult))
		{
			Editor.getProperties().getHeader().setDescription(stringResult.getValue());
			frame.documentIsModified = true;
		}
	}
} //  @jve:decl-index=0:visual-constraint="10,10"
