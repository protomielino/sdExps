/*
 *   PitProperties.java
 *   Created on 27 ??? 2005
 *
 *    The PitProperties.java is part of TrackEditor-0.3.1.
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

import java.util.Iterator;
import java.util.Vector;

import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JTextField;

import gui.EditorFrame;
import utils.Editor;
import utils.TrackData;
import utils.circuit.Segment;
import utils.circuit.SegmentSide;

/**
 * @author babis
 *
 * TODO To change the template for this generated type comment go to Window -
 * Preferences - Java - Code Style - Code Templates
 */
public class PitProperties extends PropertyPanel
{
	private JLabel				styleLabel				= new JLabel();
	private JComboBox<String>	styleComboBox			= null;
	private JLabel				sideLabel				= new JLabel();
	private JComboBox<String>	sideComboBox			= null;
	private JLabel				entryLabel				= new JLabel();
	private JTextField			entryTextField			= new JTextField();
	private JLabel				startLabel				= new JLabel();
	private JTextField			startTextField			= new JTextField();
	private JLabel				startBuildingsLabel		= new JLabel();
	private JTextField			startBuildingsTextField	= new JTextField();
	private JLabel				stopBuildingsLabel		= new JLabel();
	private JTextField			stopBuildingsTextField	= new JTextField();
	private JLabel				maxPitsLabel			= new JLabel();
	private JTextField			maxPitsTextField		= new JTextField();
	private JLabel				endLabel				= new JLabel();
	private JTextField			endTextField			= new JTextField();
	private JLabel				exitLabel				= new JLabel();
	private JTextField			exitTextField			= new JTextField();
	private JLabel				widthLabel				= new JLabel();
	private JTextField			widthTextField			= new JTextField();
	private JLabel				lengthLabel				= new JLabel();
	private JTextField			lengthTextField			= new JTextField();
	private JLabel				indicatorLabel			= new JLabel();
	private JComboBox<String>	indicatorComboBox		= null;
	private JLabel				speedLimitLabel			= new JLabel();
	private JTextField			speedLimitTextField		= new JTextField();
	private JLabel 				generatePitsLabel 		= new JLabel();
	private JCheckBox 			generatePitsCheckBox 	= null;
	private boolean 			generatePits			= false;

	/**
	 *
	 */
	public PitProperties(EditorFrame frame)
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

		addLabel(this, 0, styleLabel, "Type", 110);
		addLabel(this, 1, sideLabel, "Side", 110);
		addLabel(this, 2, entryLabel, "Entry", 110);
		addLabel(this, 3, startLabel, "Start", 110);
		addLabel(this, 4, startBuildingsLabel, "Start Buildings", 110);
		addLabel(this, 5, stopBuildingsLabel, "Stop Buildings", 110);
		addLabel(this, 6, maxPitsLabel, "Max Pits", 110);
		addLabel(this, 7, endLabel, "End", 110);
		addLabel(this, 8, exitLabel, "Exit", 110);
		addLabel(this, 9, widthLabel, "Width", 110);
		addLabel(this, 10, lengthLabel, "Length", 110);
		addLabel(this, 11, indicatorLabel, "Indicator", 110);
		addLabel(this, 12, speedLimitLabel, "Speed Limit", 110);
		addLabel(this, 0, generatePitsLabel, "Generate Pits", 280, 100);

		add(getStyleComboBox(), null);
		add(getSideComboBox(), null);

		addTextField(this, 2, entryTextField, Editor.getProperties().getMainTrack().getPits().getEntry(), 120, 100);
		addTextField(this, 3, startTextField, Editor.getProperties().getMainTrack().getPits().getStart(), 120, 100);
		addTextField(this, 4, startBuildingsTextField, Editor.getProperties().getMainTrack().getPits().getStartBuildings(), 120, 100);
		addTextField(this, 5, stopBuildingsTextField, Editor.getProperties().getMainTrack().getPits().getStopBuildings(), 120, 100);
		addTextField(this, 6, maxPitsTextField, Editor.getProperties().getMainTrack().getPits().getMaxPits(), 120, 100);
		addTextField(this, 7, endTextField, Editor.getProperties().getMainTrack().getPits().getEnd(), 120, 100);
		addTextField(this, 8, exitTextField, Editor.getProperties().getMainTrack().getPits().getExit(), 120, 100);
		addTextField(this, 9, widthTextField, Editor.getProperties().getMainTrack().getPits().getWidth(), 120, 40);
		addTextField(this, 10, lengthTextField, Editor.getProperties().getMainTrack().getPits().getLength(), 120, 40);

		add(getIndicatorComboBox(), null);

		addTextField(this, 12, speedLimitTextField, Editor.getProperties().getMainTrack().getPits().getSpeedLimit(), 120, 40);

		add(getGeneratePitsCheckBox(), null);
	}
	/**
	 * This method initializes styleComboBox
	 *
	 * @return javax.swing.JComboBox
	 */
	private JComboBox<String> getStyleComboBox()
	{
		if (styleComboBox == null)
		{
			String[] items = {"none", "no pits", "on track side", "on separate path", "no building"};
			styleComboBox = new JComboBox<String>(items);
			styleComboBox.setBounds(120, 10, 120, 23);
			int style = Editor.getProperties().getMainTrack().getPits().getStyle();
			if (style == Integer.MAX_VALUE)
				style = 0;
			else
				style++;
			styleComboBox.setSelectedIndex(style);
		}
		return styleComboBox;
	}
	/**
	 * This method initializes sideComboBox
	 *
	 * @return javax.swing.JComboBox
	 */
	private JComboBox<String> getSideComboBox()
	{
		if (sideComboBox == null)
		{
			String[] items = {"none", "right", "left"};
			sideComboBox = new JComboBox<String>(items);
			sideComboBox.setBounds(120, 37, 80, 23);
			String side = Editor.getProperties().getMainTrack().getPits().getSide();
			if (side == null || side.isEmpty())
				side = "none";
			sideComboBox.setSelectedItem(side);
		}
		return sideComboBox;
	}

	/**
	 * This method initializes indicatorComboBox
	 *
	 * @return javax.swing.JComboBox
	 */
	private JComboBox<String> getIndicatorComboBox()
	{
		if (indicatorComboBox == null)
		{
			String[] items = {"none", "no", "yes"};
			indicatorComboBox = new JComboBox<String>(items);
			indicatorComboBox.setBounds(120, 307, 80, 23);
			int indicator = Editor.getProperties().getMainTrack().getPits().getIndicator();
			if (indicator == Integer.MAX_VALUE)
				indicator = 0;
			else
				indicator++;
			indicatorComboBox.setSelectedIndex(indicator);
		}
		return indicatorComboBox;
	}

	/**
	 *
	 */
	public void exit()
	{
		int index = getStyleComboBox().getSelectedIndex();
		int style = Editor.getProperties().getMainTrack().getPits().getStyle();
		if (index == 0)
		{
			if (style != Integer.MAX_VALUE)
			{
				Editor.getProperties().getMainTrack().getPits().setStyle(Integer.MAX_VALUE);
				frame.documentIsModified = true;
			}
		}
		else if (style == Integer.MAX_VALUE || style != index - 1)
		{
			Editor.getProperties().getMainTrack().getPits().setStyle(index - 1);
			frame.documentIsModified = true;
		}

		MutableString stringResult = new MutableString();
		MutableDouble doubleResult = new MutableDouble();
		MutableInteger integerResult = new MutableInteger();

		if (isDifferent((String) getSideComboBox().getSelectedItem(),
			Editor.getProperties().getMainTrack().getPits().getSide(), stringResult))
		{
			Editor.getProperties().getMainTrack().getPits().setSide(stringResult.getValue());
			frame.documentIsModified = true;
		}

		if (isDifferent(entryTextField.getText(),
			Editor.getProperties().getMainTrack().getPits().getEntry(), stringResult))
		{
			Editor.getProperties().getMainTrack().getPits().setEntry(stringResult.getValue());
			frame.documentIsModified = true;
		}

		if (isDifferent(startTextField.getText(),
			Editor.getProperties().getMainTrack().getPits().getStart(), stringResult))
		{
			Editor.getProperties().getMainTrack().getPits().setStart(stringResult.getValue());
			frame.documentIsModified = true;
		}

		if (isDifferent(startBuildingsTextField.getText(),
			Editor.getProperties().getMainTrack().getPits().getStartBuildings(), stringResult))
		{
			Editor.getProperties().getMainTrack().getPits().setStartBuildings(stringResult.getValue());
			frame.documentIsModified = true;
		}

		if (isDifferent(stopBuildingsTextField.getText(),
			Editor.getProperties().getMainTrack().getPits().getStopBuildings(), stringResult))
		{
			Editor.getProperties().getMainTrack().getPits().setStopBuildings(stringResult.getValue());
			frame.documentIsModified = true;
		}

		if (isDifferent(maxPitsTextField.getText(),
			Editor.getProperties().getMainTrack().getPits().getMaxPits(), integerResult))
		{
			Editor.getProperties().getMainTrack().getPits().setMaxPits(integerResult.getValue());
			frame.documentIsModified = true;
		}

		if (isDifferent(endTextField.getText(),
			Editor.getProperties().getMainTrack().getPits().getEnd(), stringResult))
		{
			Editor.getProperties().getMainTrack().getPits().setEnd(stringResult.getValue());
			frame.documentIsModified = true;
		}

		if (isDifferent(exitTextField.getText(),
			Editor.getProperties().getMainTrack().getPits().getExit(), stringResult))
		{
			Editor.getProperties().getMainTrack().getPits().setExit(stringResult.getValue());
			frame.documentIsModified = true;
		}

		if (isDifferent(widthTextField.getText(),
			Editor.getProperties().getMainTrack().getPits().getWidth(), doubleResult))
		{
			Editor.getProperties().getMainTrack().getPits().setWidth(doubleResult.getValue());
			frame.documentIsModified = true;
		}

		if (isDifferent(lengthTextField.getText(),
			Editor.getProperties().getMainTrack().getPits().getLength(), doubleResult))
		{
			Editor.getProperties().getMainTrack().getPits().setLength(doubleResult.getValue());
			frame.documentIsModified = true;
		}

		index = getIndicatorComboBox().getSelectedIndex();
		int indicator = Editor.getProperties().getMainTrack().getPits().getIndicator();
		if (index == 0)
		{
			if (indicator != Integer.MAX_VALUE)
			{
				Editor.getProperties().getMainTrack().getPits().setIndicator(Integer.MAX_VALUE);
				frame.documentIsModified = true;
			}
		}
		else if (indicator == Integer.MAX_VALUE || indicator != index - 1)
		{
			Editor.getProperties().getMainTrack().getPits().setIndicator(index - 1);
			frame.documentIsModified = true;
		}

		if (isDifferent(speedLimitTextField.getText(),
			Editor.getProperties().getMainTrack().getPits().getSpeedLimit(), doubleResult))
		{
			Editor.getProperties().getMainTrack().getPits().setSpeedLimit(doubleResult.getValue());
			frame.documentIsModified = true;
		}

		if (getGeneratePitsCheckBox().isSelected())
		{
			createPits();
			frame.documentIsModified = true;
		}
	}

	/**
	 *
	 */
	private void createPits()
	{
		Vector<Segment> data = TrackData.getTrackData();
		Segment pitEntry = null;
		Segment pitStart = null;
		Segment pitStartBuildings = null;
		Segment pitStopBuildings = null;
		Segment pitEnd = null;
		Segment pitExit = null;

		Iterator<Segment> it = data.iterator();
		while (it.hasNext())
		{
			Segment obj = it.next();
			String name = obj.getName();
			if (name.equals(Editor.getProperties().getMainTrack().getPits().getEntry()))
			{
				pitEntry = obj;
			}else if (name.equals(Editor.getProperties().getMainTrack().getPits().getStart()))
			{
				pitStart = obj;
			}else if (name.equals(Editor.getProperties().getMainTrack().getPits().getStartBuildings()))
			{
				pitStartBuildings = obj;
			}else if (name.equals(Editor.getProperties().getMainTrack().getPits().getStopBuildings()))
			{
				pitStopBuildings = obj;
			}else if (name.equals(Editor.getProperties().getMainTrack().getPits().getEnd()))
			{
				pitEnd = obj;
			}else if (name.equals(Editor.getProperties().getMainTrack().getPits().getExit()))
			{
				pitExit = obj;
			}
		}
		SegmentSide side = null;
		if(pitEntry == null)
		{
			System.out.println("No pit entry");
			return;
		}
		if(Editor.getProperties().getMainTrack().getPits().getSide().equals("left"))
		{
			side = pitEntry.getLeft();
		}else
		{
			side = pitEntry.getRight();
		}
		side.setBorderHeight(0);
		side.setBorderWidth(0);
		side.setSideEndWidth(Editor.getProperties().getMainTrack().getPits().getWidth()*3);
		side.setSideSurface("road1");
		side.setBarrierHeight(1);
		side.setBarrierWidth(0.1);

		if(pitExit == null)
		{
			System.out.println("No pit exit");
			return;
		}
		if(Editor.getProperties().getMainTrack().getPits().getSide().equals("left"))
		{
			side = pitExit.getLeft();
		}else
		{
			side = pitExit.getRight();
		}
		side.setBorderHeight(0);
		side.setBorderWidth(0);
		side.setSideStartWidth(Editor.getProperties().getMainTrack().getPits().getWidth()*3);
		side.setSideSurface("road1");
		side.setBarrierHeight(1);
		side.setBarrierWidth(0.1);

		if(pitStart == null || pitEnd == null)
		{
			System.out.println("No pit start or end");
			return;
		}

		int start = data.indexOf(pitEntry);
		int end = data.indexOf(pitExit);

		for(int i=start+1; i<data.size(); i++)
		{
			if(Editor.getProperties().getMainTrack().getPits().getSide().equals("left"))
			{
				side = ((Segment) data.get(i)).getLeft();
			}else
			{
				side = ((Segment) data.get(i)).getRight();
			}
			side.setBorderHeight(1);
			side.setBorderWidth(0.1);
			side.setSideStartWidth(Editor.getProperties().getMainTrack().getPits().getWidth()*3);
			side.setSideEndWidth(Editor.getProperties().getMainTrack().getPits().getWidth()*3);
			side.setSideSurface("road1-pits");
			side.setBarrierHeight(1);
			side.setBarrierWidth(0.1);
		}

		for(int i=0; i<end; i++)
		{
			if(Editor.getProperties().getMainTrack().getPits().getSide().equals("left"))
			{
				side = ((Segment) data.get(i)).getLeft();
			}else
			{
				side = ((Segment) data.get(i)).getRight();
			}
			side.setBorderHeight(1);
			side.setBorderWidth(0.1);
			side.setSideStartWidth(Editor.getProperties().getMainTrack().getPits().getWidth()*3);
			side.setSideEndWidth(Editor.getProperties().getMainTrack().getPits().getWidth()*3);
			side.setSideSurface("road1-pits");
			side.setBarrierHeight(1);
			side.setBarrierWidth(0.1);
		}
	}

	/**
	 * This method initializes generatePitsCheckBox
	 *
	 * @return javax.swing.JCheckBox
	 */
	private JCheckBox getGeneratePitsCheckBox() {
		if (generatePitsCheckBox == null) {
			generatePitsCheckBox = new JCheckBox();
			generatePitsCheckBox.setBounds(388, 10, 20, 20);
			generatePitsCheckBox.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e)
				{
					if(generatePitsCheckBox.isSelected())
					{
						generatePits = true;
					}else
					{
						generatePits = false;
					}
				}
			});
		}
		return generatePitsCheckBox;
	}
 } //  @jve:decl-index=0:visual-constraint="10,10"
