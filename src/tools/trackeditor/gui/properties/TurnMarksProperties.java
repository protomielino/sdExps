/*
 *   TurnMarksProperties.java
 *   Created on 31 May 2022
 *
 *    The TurnMarksProperties.java is part of TrackEditor-0.7.0.
 *
 *    TrackEditor-0.7.0 is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    TrackEditor-0.7.0 is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with TrackEditor-0.7.0; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
package gui.properties;

import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JTextField;

import gui.EditorFrame;
import utils.Editor;
import utils.circuit.TurnMarks;

/**
 * @author Robert Reif
 *
 * TODO To change the template for this generated type comment go to Window -
 * Preferences - Java - Code Style - Code Templates
 */
public class TurnMarksProperties extends PropertyPanel
{
	private JLabel		widthLabel					= new JLabel();
	private JTextField	widthTextField				= new JTextField();
	private JLabel		heightLabel					= new JLabel();
	private JTextField	heightTextField				= new JTextField();
	private JLabel		verticalSpaceLabel			= new JLabel();
	private JTextField	verticalSpaceTextField		= new JTextField();
	private JLabel		horizontalSpaceLabel		= new JLabel();
	private JTextField	horizontalSpaceTextField	= new JTextField();
	private JButton		defaultButton				= null;
	private JButton		deleteButton				= null;

	/**
	 *
	 */
	public TurnMarksProperties(EditorFrame frame)
	{
		super(frame);
		initialize();
	}

	/**
	 * This method initializes this
	 *
	 * @return void
	 */
	private void initialize()
	{
		setLayout(null);
		setBorder(javax.swing.BorderFactory.createEtchedBorder(javax.swing.border.EtchedBorder.LOWERED));

		addLabel(this, 0, widthLabel, "Width", 120);
		addLabel(this, 1, heightLabel, "Height", 120);
		addLabel(this, 2, verticalSpaceLabel, "Vertical Space", 120);
		addLabel(this, 3, horizontalSpaceLabel, "Horizontal Space", 120);

		addTextField(this, 0, widthTextField, Editor.getProperties().getGraphic().getTurnMarks().getWidth(), 130, 100);
		addTextField(this, 1, heightTextField, Editor.getProperties().getGraphic().getTurnMarks().getHeight(), 130, 100);
		addTextField(this, 2, verticalSpaceTextField, Editor.getProperties().getGraphic().getTurnMarks().getVerticalSpace(), 130, 100);
		addTextField(this, 3, horizontalSpaceTextField, Editor.getProperties().getGraphic().getTurnMarks().getHorizontalSpace(), 130, 100);

		add(getDefaultButton(), null);
		add(getDeleteButton(), null);
	}

	/**
	 * This method initializes defaultButton
	 *
	 * @return javax.swing.JButton
	 */
	private JButton getDefaultButton()
	{
		if (defaultButton == null)
		{
			defaultButton = new JButton();
			defaultButton.setBounds(300, 15, 80, 25);
			defaultButton.setText("Default");
			defaultButton.addActionListener(new java.awt.event.ActionListener()
			{
				public void actionPerformed(java.awt.event.ActionEvent e)
				{
					widthTextField.setText(TurnMarks.DEFAULT_WIDTH + "");
					heightTextField.setText(TurnMarks.DEFAULT_HEIGHT + "");
					verticalSpaceTextField.setText(TurnMarks.DEFAULT_VERTICAL_SPACE + "");
					horizontalSpaceTextField.setText(TurnMarks.DEFAULT_HORIZONTAL_SPACE + "");
				}
			});
		}
		return defaultButton;
	}
	/**
	 * This method initializes deleteButton
	 *
	 * @return javax.swing.JButton
	 */
	private JButton getDeleteButton()
	{
		if (deleteButton == null)
		{
			deleteButton = new JButton();
			deleteButton.setBounds(300, 50, 80, 25);
			deleteButton.setText("Delete");
			deleteButton.addActionListener(new java.awt.event.ActionListener()
			{
				public void actionPerformed(java.awt.event.ActionEvent e)
				{
					widthTextField.setText("");
					heightTextField.setText("");
					verticalSpaceTextField.setText("");
					horizontalSpaceTextField.setText("");
				}
			});
		}
		return deleteButton;
	}

	/**
	 *
	 */
	public void exit()
	{
		MutableDouble doubleResult = new MutableDouble();

		if (isDifferent(widthTextField.getText(),
			Editor.getProperties().getGraphic().getTurnMarks().getWidth(), doubleResult))
		{
			Editor.getProperties().getGraphic().getTurnMarks().setWidth(doubleResult.getValue());
			frame.documentIsModified = true;
		}

		if (isDifferent(heightTextField.getText(),
			Editor.getProperties().getGraphic().getTurnMarks().getHeight(), doubleResult))
		{
			Editor.getProperties().getGraphic().getTurnMarks().setHeight(doubleResult.getValue());
			frame.documentIsModified = true;
		}

		if (isDifferent(verticalSpaceTextField.getText(),
			Editor.getProperties().getGraphic().getTurnMarks().getVerticalSpace(), doubleResult))
		{
			Editor.getProperties().getGraphic().getTurnMarks().setVerticalSpace(doubleResult.getValue());
			frame.documentIsModified = true;
		}

		if (isDifferent(horizontalSpaceTextField.getText(),
			Editor.getProperties().getGraphic().getTurnMarks().getHorizontalSpace(), doubleResult))
		{
			Editor.getProperties().getGraphic().getTurnMarks().setHorizontalSpace(doubleResult.getValue());
			frame.documentIsModified = true;
		}
	}
} //  @jve:decl-index=0:visual-constraint="10,10"
