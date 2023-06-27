package gui.view;

import gui.EditorFrame;
import gui.TrackObjectDialog;
import gui.segment.SegmentEditorDlg;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.event.MouseWheelEvent;
import java.awt.event.MouseWheelListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Vector;

import javax.imageio.ImageIO;
import javax.swing.AbstractAction;
import javax.swing.ImageIcon;
import javax.swing.JComponent;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.event.PopupMenuEvent;
import javax.swing.event.PopupMenuListener;

import utils.Editor;
import utils.EditorPoint;
import utils.SegmentVector;
import utils.circuit.Curve;
import utils.circuit.ObjShapeHandle;
import utils.circuit.ObjShapeObject;
import utils.circuit.ObjShapeRelief;
import utils.circuit.ObjShapeTerrain;
import utils.circuit.ObjectMap;
import utils.circuit.Reliefs;
import utils.circuit.Segment;
import utils.circuit.Straight;
import utils.circuit.XmlObjPits;
import utils.undo.ObjectMapObject;
import utils.undo.Undo;
import utils.undo.UndoAddObject;
import utils.undo.UndoAddSegment;
import utils.undo.UndoDeleteAllObjects;
import utils.undo.UndoDeleteObject;
import utils.undo.UndoDeleteRelief;
import utils.undo.UndoDeleteSegment;
import utils.undo.UndoEditAllObjects;
import utils.undo.UndoEditObject;
import utils.undo.UndoEditRelief;
import utils.undo.UndoSegmentChange;
import utils.undo.UndoSplitSegment;


/**
 * <p>
 * Titre : Torcs Tune
 * </p>
 * <p>
 * Description : Torcs tuning
 * </p>
 * <p>
 * Copyright : Copyright (c) 2002 Patrice Espie
 * </p>
 * <p>
 * Soci�t� :
 * </p>
 * 
 * @author Patrice Espie
 * @version 0.1a
 */

public class CircuitView extends JComponent implements KeyListener, MouseListener, MouseMotionListener, WindowListener, MouseWheelListener
{
	/** zooming factor */
	double						zoomFactor						= 1.0;
	/** affine transformation */
	AffineTransform				affineTransform					= new AffineTransform();
	AffineTransform				inverseAffineTransform			= new AffineTransform();

	/** xml document containing current circuit */
	//	EPXMLDocument circuit;
	/** bounding rectangle of all elements of the circuits, in meters */
	Rectangle2D.Double			boundingRectangle				= new Rectangle2D.Double(0, 0, 0, 0);
	/** width of zone outside circuit terrain, shown on screen */
	double						outZoneWidth					= 100;									// meters
	/** height of zone outside circuit terrain, shown on screen */
	double						outZoneHeight					= 100;									// meters
	/** situation on circuit terrain of the screen center */
	Point2D.Double				screenCenter					= new Point2D.Double(0, 0);			// meters

	/** pits shape */
	XmlObjPits					pits;
	/** terrain shape */
	ObjShapeTerrain				terrain;

	/** event to fire when selection has changed */
	CircuitViewSelectionEvent	selectionChangedEvent			= new CircuitViewSelectionEvent(this);
	/** current selected shape */
	public Segment				selectedShape					= null;
	/** current handled shape */
	public Segment				handledShape					= null;	
	/** current dragging state */
	boolean						dragging						= false;

	/** current object dragging state */
	boolean						objectDragging					= false;
	/** current object shape */
	private ObjShapeObject		objectShape						= null;
	/** offset between mouse click position and object position */
	private Point2D.Double		objectOffset					= new Point2D.Double();
	
	private ObjShapeRelief		reliefShape						= null;
	private int					reliefPointIndex				= 0;
	private boolean				showReliefs						= false;
	private boolean				reliefDragging					= false;
	private Point2D.Double		reliefOffset					= new Point2D.Double();

	/** mouse pressed point, in meters */
	Point2D.Double				clickPoint						= new Point2D.Double(0, 0);
	/** mouse current point, in meters */
	Point2D.Double				mousePoint						= new Point2D.Double(0, 0);
	/** handles to be shown */
	ArrayList<Segment>			handles							= new ArrayList<Segment>();
	/** temp handle for calculs */
	ObjShapeHandle				handle							= new ObjShapeHandle();
	/** dragging handle index */
	int							handleDragging					= -1;
	/** current moved shape, for undo management */
	Segment						currentMovedShape				= null;

	/** flag for skipping recurrent events */
	boolean						isAncestorResizing				= false;

	/** UI dialog */
	public SegmentEditorDlg		segmentParamDialog;

	/** current operating state */
	int							currentState;

	/** operating states */
	static public final int		STATE_NONE						= 0;
	static public final int		STATE_CREATE_STRAIGHT			= 1;
	static public final int		STATE_CREATE_LEFT_SEGMENT		= 2;
	static public final int		STATE_CREATE_RIGHT_SEGMENT		= 3;
	static public final int		STATE_DELETE					= 4;
	static public final int		STATE_SHOW_BGRD_START_POSITION	= 5;
	static public final int		STATE_MOVE_SEGMENTS				= 6;
	static public final int		STATE_SUBDIVIDE					= 7;
	static public final int		STATE_FINISH_LINE				= 8;
//	static public final int		STATE_SHOW_OBJECTS				= 9;
//	static public final int		STATE_SHOW_RELIEFSS				= 10;

	/** arrow showing state */
	public boolean				showArrows						= false;

	/** undo list */
	protected ArrayList<CircuitViewSelectionListener>	undoSteps	= new ArrayList<CircuitViewSelectionListener>();
	/** redo list */
	protected ArrayList<CircuitViewSelectionListener>	redoSteps	= new ArrayList<CircuitViewSelectionListener>();

	/** show terrain border */
	boolean						terrainBorderMustBeShown		= true;

	/** selection listener management */
	private transient Vector<CircuitViewSelectionListener>	selectionListeners;

	//private double				imgCo							= 1.0; //3.4;
	//private Point2D.Double		imgOffset						= new Point2D.Double(0, 0);
	private EditorPoint			imgOffsetPrev					= new EditorPoint();
	private EditorPoint			imgOffsetStart					= new EditorPoint();
	/** background image */
	ImageIcon					backgroundImg					= null;
	/** background image position, in meters */
	Rectangle2D.Double			backgroundRectangle				= new Rectangle2D.Double();
	/** background image showing state */
	public boolean				showBackground					= true;

	/** object showing state */
	public boolean				showObjects						= false;

	private int 				currentCount					= 0;

	/** upward link to parent frame */
	EditorFrame					editorFrame;

	Graphics					graphics						= null;

	private final String sep = System.getProperty("file.separator");

	/**
	 * constructor
	 * 
	 * @param editorFrame
	 *            Upward link
	 */
	public CircuitView(EditorFrame editorFrame)
	{
		try
		{
			addKeyListener(this);
			addMouseListener(this);
			addMouseMotionListener(this);
			addMouseWheelListener(this);
			this.editorFrame = editorFrame;
			terrain = new ObjShapeTerrain();
			Editor.getProperties().addPropertiesListener(new ActionListener()
			{

				public void actionPerformed(ActionEvent e)
				{
					setCircuit();
				}

			});

			jbInit();
		} catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	/**
	 *  
	 */
	protected void setCircuit()
	{
		//		System.out.println("CircuitView.setCircuit");
		//		terrain = new ObjShapeTerrain();
		//		terrain.calcShape(null, boundingRectangle);
	}

	/**
	 * blank constructor
	 */
	public CircuitView()
	{
		this(null);
	}

	public Rectangle2D.Double getBoundingRectangle()
	{
		return boundingRectangle;
	}
	
	/**
	 * Identifies whether or not this component can receive the focus.
	 * 
	 * @return
	 */
	public boolean isFocusTraversable()
	{
		return true;
	}

	public void screenToReal(MouseEvent e, Point2D.Double point)
	{
		point.setLocation(e.getX(), e.getY());
		inverseAffineTransform.transform(point, point);
	}

	/** input events management */
	public void keyTyped(KeyEvent e)
	{
	}
	/** input events management */
	public void keyPressed(KeyEvent e)
	{
	}
	/** input events management */
	public void keyReleased(KeyEvent e)
	{
	}

	/** input events management */
	public void mouseClicked(MouseEvent e)
	{
		if (e.getButton() == MouseEvent.BUTTON1)
		{
			screenToReal(e, clickPoint);
			screenToReal(e, mousePoint);

			try
			{
				switch (currentState)
				{
					case STATE_NONE :
					{
					}
						break;

					case STATE_CREATE_LEFT_SEGMENT :
					{
						if (handledShape == null)
							return;

						// create a standard curve segment
						SegmentVector data = editorFrame.getTrackData().getSegments();
						int pos = data.indexOf(handledShape);
						Curve newShape = new Curve("lft",handledShape);
						newShape.setArcDeg(90);
						newShape.setRadiusStart(50);
						newShape.setRadiusEnd(50);
						newShape.inheritProperties(handledShape);
						int count = Editor.getProperties().getCurveNameCount()+1;
						Editor.getProperties().setCurveNameCount(count);
						newShape.setName("curve "+count);
						data.insertElementAt(newShape,pos+1);
						newShape.setCount(data.indexOf(newShape) + 1);
						Undo.add(new UndoAddSegment(editorFrame, newShape));
						selectedShape = newShape;
						openSegmentDialog(newShape);
						segmentParamDialog.addWindowListener(this);
						editorFrame.documentIsModified = true;
						this.redrawCircuit();
					}
						break;
					case STATE_CREATE_RIGHT_SEGMENT :
					{
						if (handledShape == null)
							return;

						// create a standard curve segment
						SegmentVector data = editorFrame.getTrackData().getSegments();
						int pos = data.indexOf(handledShape);
						Curve newShape = new Curve("rgt",handledShape);
						newShape.setArcDeg(90);
						newShape.setRadiusStart(50);
						newShape.setRadiusEnd(50);
						newShape.inheritProperties(handledShape);
						int count = Editor.getProperties().getCurveNameCount()+1;
						Editor.getProperties().setCurveNameCount(count);
						newShape.setName("curve "+count);
						data.insertElementAt(newShape,pos+1);
						newShape.setCount(data.indexOf(newShape) + 1);
						Undo.add(new UndoAddSegment(editorFrame, newShape));
						selectedShape = newShape;
						openSegmentDialog(newShape);
						segmentParamDialog.addWindowListener(this);
						editorFrame.documentIsModified = true;
						this.redrawCircuit();
					}
						break;

					case STATE_CREATE_STRAIGHT :
					{
						if (handledShape == null)
							return;

						// create a standard straight segment
						SegmentVector data = editorFrame.getTrackData().getSegments();
						int pos = data.indexOf(handledShape);
						Straight newShape = new Straight();
						newShape.setLength(50);
						newShape.inheritProperties(handledShape);
						int count = Editor.getProperties().getStraightNameCount()+1;
						Editor.getProperties().setStraightNameCount(count);
						newShape.setName("straight "+count);
						data.insertElementAt(newShape, pos + 1);
						newShape.setCount(data.indexOf(newShape) + 1);
						Undo.add(new UndoAddSegment(editorFrame, newShape));
						selectedShape = newShape;
						openSegmentDialog(newShape);
						segmentParamDialog.addWindowListener(this);
						editorFrame.documentIsModified = true;
						this.redrawCircuit();
					}
						break;

					case STATE_DELETE :
					{
						if (handledShape == null)
							return;

						try
						{
							boolean mustFireEvent = (selectedShape != null);

							selectedShape = null;

							if (mustFireEvent)
								fireSelectionChanged(selectionChangedEvent);

							if (segmentParamDialog != null)
							{
								if (!segmentParamDialog.dirty)
								{
									//popUndo();
								}

								segmentParamDialog.dispose();
								segmentParamDialog = null;
							}

							// must check for a segment under the mouse
							SegmentVector data = editorFrame.getTrackData().getSegments();
							int pos = data.indexOf(handledShape);
							Undo.add(new UndoDeleteSegment(editorFrame, handledShape));
							data.remove(pos);
							handledShape = null;
							//selectedShape = newShape;
							
							editorFrame.documentIsModified = true;

							this.redrawCircuit();

							setState(STATE_NONE);
							editorFrame.toggleButtonDelete.setSelected(false);

							editorFrame.documentIsModified = true;
						} catch (Exception ex)
						{
							ex.printStackTrace();
						}
					}
						break;

					case STATE_SUBDIVIDE:
					{
						if (handledShape == null)
						{
							return;
						}
						SegmentVector data = editorFrame.getTrackData().getSegments();
						int pos = data.indexOf(handledShape);
						double splitPoint = 0.5;
						switch (handledShape.getType())
						{
							case "lft":
							{
								Curve oldShape = (Curve)handledShape;
								Curve cloneShape = (Curve)oldShape.clone();
								Curve newShape2 = new Curve("lft", oldShape);
								double arc = oldShape.getArcDeg();
								double rStart = oldShape.getRadiusStart();
								double rEnd = oldShape.getRadiusEnd();
								double L = (rStart + rEnd) / 2.0 * oldShape.getArcDeg();
								double nbSteps = (int)(L / 4.0 + 0.5) + 1;
								double stepLength = L / nbSteps;
								double deltaRadiusStep = (rEnd - rStart) / (nbSteps - 1.0);
								double tmpAngle = 0.0;
								double tmpRadius = rStart;
								for (int curStep = 0; curStep < nbSteps; ++curStep)
								{
									tmpAngle += stepLength / tmpRadius;
									tmpRadius += deltaRadiusStep;
								}
								stepLength *= arc / tmpAngle;
								tmpAngle = 0.0;
								tmpRadius = rStart;
								for (int curStep = 0; curStep < nbSteps; ++curStep)
								{
									tmpAngle += stepLength / tmpRadius;
									tmpRadius += deltaRadiusStep;
									oldShape.setArcDeg(tmpAngle);
									oldShape.setRadiusEnd(tmpRadius - deltaRadiusStep);
									redrawCircuit();
									if (oldShape.contains(mousePoint))
									{
										splitPoint = curStep / nbSteps;
										break;
									}
								}
								newShape2.setArcDeg(arc - tmpAngle);
								newShape2.setRadiusEnd(rEnd);
								newShape2.setRadiusStart(tmpRadius);
								splitSegment(oldShape, newShape2, splitPoint);
								newShape2.setProfilStepsLength(4.0);
								int count2 = Editor.getProperties().getCurveNameCount() + 1;
								Editor.getProperties().setCurveNameCount(count2);
								newShape2.setName("curve " + count2);
								data.insertElementAt(newShape2, pos + 1);
								Undo.add(new UndoSplitSegment(editorFrame, oldShape, cloneShape, newShape2));
								break;
							}
							case "rgt":
							{
								Curve oldShape = (Curve)this.handledShape;
								Curve cloneShape = (Curve)oldShape.clone();
								Curve newShape2 = new Curve("rgt", oldShape);
								double arc = oldShape.getArcDeg();
								double rStart = oldShape.getRadiusStart();
								double rEnd = oldShape.getRadiusEnd();
								double L = (rStart + rEnd) / 2.0 * oldShape.getArcDeg();
								double nbSteps = (int)(L / 4.0 + 0.5) + 1;
								double stepLength = L / nbSteps;
								final double deltaRadiusStep = (rEnd - rStart) / (nbSteps - 1.0);
								double tmpAngle = 0.0;
								double tmpRadius = rStart;
								for (int curStep = 0; curStep < nbSteps; ++curStep)
								{
									tmpAngle += stepLength / tmpRadius;
									tmpRadius += deltaRadiusStep;
								}
								stepLength *= arc / tmpAngle;
								tmpAngle = 0.0;
								tmpRadius = rStart;
								for (int curStep = 0; curStep < nbSteps; ++curStep)
								{
									tmpAngle += stepLength / tmpRadius;
									tmpRadius += deltaRadiusStep;
									oldShape.setArcDeg(tmpAngle);
									oldShape.setRadiusEnd(tmpRadius - deltaRadiusStep);
									redrawCircuit();
									if (oldShape.contains(mousePoint))
									{
										splitPoint = curStep / nbSteps;
										break;
									}
								}
								newShape2.setArcDeg(arc - tmpAngle);
								newShape2.setRadiusEnd(rEnd);
								newShape2.setRadiusStart(tmpRadius);
								splitSegment(oldShape, newShape2, splitPoint);
								newShape2.setProfilStepsLength(4.0);
								int count2 = Editor.getProperties().getCurveNameCount() + 1;
								Editor.getProperties().setCurveNameCount(count2);
								newShape2.setName("curve " + count2);
								data.insertElementAt(newShape2, pos + 1);
								Undo.add(new UndoSplitSegment(editorFrame, oldShape, cloneShape, newShape2));
								break;
							}
							case "str":
							{
								Straight oldShape = (Straight)this.handledShape;
								Straight cloneShape = (Straight)oldShape.clone();
								Straight newShape3 = new Straight();
								double length = oldShape.getLength();
								for (int i = 0; i < 100; ++i)
								{
									splitPoint = i * 0.01;
									newShape3.setLength(length * (1.0 - splitPoint));
									oldShape.setLength(length * splitPoint);
									redrawCircuit();
									if (oldShape.contains(mousePoint))
									{
										break;
									}
								}
								splitSegment(oldShape, newShape3, splitPoint);
								int count3 = Editor.getProperties().getStraightNameCount() + 1;
								Editor.getProperties().setStraightNameCount(count3);
								newShape3.setName("straight " + count3);
								data.insertElementAt(newShape3, pos + 1);
								Undo.add(new UndoSplitSegment(editorFrame, oldShape, cloneShape, newShape3));
								break;
							}
							default:
								break;
						}
						handledShape = null;
						editorFrame.toggleButtonSubdivide.setSelected(false);
						editorFrame.documentIsModified = true;
						redrawCircuit();
						setState(STATE_NONE);
					}
						break;
					case STATE_FINISH_LINE :
					{
						if (handledShape == null)
							return;

						boolean mustFireEvent = (selectedShape != null);

						selectedShape = null;

						if (mustFireEvent)
							fireSelectionChanged(selectionChangedEvent);

						JOptionPane.showMessageDialog(this, "Not implemented yet!", "Moving Finish Line", JOptionPane.INFORMATION_MESSAGE);
						
						handledShape = null;
						editorFrame.documentIsModified = true;

						this.redrawCircuit();

						setState(STATE_NONE);
						editorFrame.finishLineToggleButton.setSelected(false);

						editorFrame.documentIsModified = true;
					}
						break;
				}
			} catch (Exception ex)
			{
				ex.printStackTrace();
			}
		}
	}

	private void splitSegment(Segment oldShape, Segment newShape, double splitPoint)
	{
		double leftSide2 = oldShape.getLeft().getSideStartWidth() + (oldShape.getLeft().getSideEndWidth() - oldShape.getLeft().getSideStartWidth()) * splitPoint;
		double rightSide2 = oldShape.getRight().getSideStartWidth() + (oldShape.getRight().getSideEndWidth() - oldShape.getRight().getSideStartWidth()) * splitPoint;

		newShape.getLeft().setBorderWidth(oldShape.getLeft().getBorderWidth());
		newShape.getRight().setBorderWidth(oldShape.getRight().getBorderWidth());
		newShape.getLeft().setBorderStyle(oldShape.getLeft().getBorderStyle());		
		newShape.getRight().setBorderStyle(oldShape.getRight().getBorderStyle());		
		newShape.getLeft().setBorderSurface(oldShape.getLeft().getBorderSurface());
		newShape.getRight().setBorderSurface(oldShape.getRight().getBorderSurface());
		newShape.getLeft().setBorderHeight(oldShape.getLeft().getBorderHeight());
		newShape.getRight().setBorderHeight(oldShape.getRight().getBorderHeight());

		newShape.getLeft().setSideStartWidth(leftSide2);
		newShape.getRight().setSideStartWidth(rightSide2);
		newShape.getLeft().setSideEndWidth(oldShape.getLeft().getSideEndWidth());
		newShape.getRight().setSideEndWidth(oldShape.getRight().getSideEndWidth());
		newShape.getLeft().setSideBankingType(oldShape.getLeft().getSideBankingType());
		newShape.getRight().setSideBankingType(oldShape.getRight().getSideBankingType());
		newShape.getLeft().setSideSurface(oldShape.getLeft().getSideSurface());
		newShape.getRight().setSideSurface(oldShape.getRight().getSideSurface());
		oldShape.getLeft().setSideEndWidth(leftSide2);
		oldShape.getRight().setSideEndWidth(rightSide2);

		newShape.getLeft().setBarrierStyle(oldShape.getLeft().getBarrierStyle());		
		newShape.getRight().setBarrierStyle(oldShape.getRight().getBarrierStyle());		
		newShape.getLeft().setBarrierSurface(oldShape.getLeft().getBarrierSurface());
		newShape.getRight().setBarrierSurface(oldShape.getRight().getBarrierSurface());
		newShape.getLeft().setBarrierHeight(oldShape.getLeft().getBarrierHeight());
		newShape.getRight().setBarrierHeight(oldShape.getRight().getBarrierHeight());
		newShape.getLeft().setBarrierWidth(oldShape.getLeft().getBarrierWidth());
		newShape.getRight().setBarrierWidth(oldShape.getRight().getBarrierWidth());

		if (!Double.isNaN(oldShape.getHeightStartLeft()) && !Double.isNaN(oldShape.getHeightEndLeft()))
		{								
			newShape.setHeightEndLeft(oldShape.getHeightEndLeft());
			double leftHeight = oldShape.getHeightStartLeft() + (oldShape.getHeightEndLeft() - oldShape.getHeightStartLeft()) * splitPoint;
			newShape.setHeightStartLeft(leftHeight);
			oldShape.setHeightEndLeft(leftHeight);
		}

		if (!Double.isNaN(oldShape.getHeightStartRight()) && !Double.isNaN(oldShape.getHeightEndRight()))
		{								
			newShape.setHeightEndRight(oldShape.getHeightEndRight());
			double rightHeight = oldShape.getHeightStartRight() + (oldShape.getHeightEndRight() - oldShape.getHeightStartRight()) * splitPoint;
			newShape.setHeightStartRight(rightHeight);
			oldShape.setHeightEndRight(rightHeight);
		}

		if (!Double.isNaN(oldShape.getBankingStart()) && !Double.isNaN(oldShape.getBankingEnd()))
		{								
			newShape.setBankingEnd(oldShape.getBankingEnd());
			double banking = oldShape.getBankingStart() + (oldShape.getBankingEnd() - oldShape.getBankingStart()) * splitPoint;
			newShape.setBankingStart(banking);
			oldShape.setBankingEnd(banking);
		}

		if (!Double.isNaN(oldShape.getProfilStartTangent()) && !Double.isNaN(oldShape.getProfilEndTangent()))
		{
			newShape.setProfilEndTangent(oldShape.getProfilEndTangent());
			// FIXME: calculate the slope at the split point									
			double tangent = oldShape.getProfilStartTangent() + (oldShape.getProfilEndTangent() - oldShape.getProfilStartTangent()) * splitPoint;
			newShape.setProfilStartTangent(tangent);
			oldShape.setProfilEndTangent(tangent);
		}

		newShape.setGrade(oldShape.getGrade());
		newShape.setSurface(oldShape.getSurface());
	}

	private ObjectMap findObjectMap(ObjShapeObject object)
	{
		for (int i = 0; i < editorFrame.getObjectMaps().size(); i++)
		{
			ObjectMap objectMap = editorFrame.getObjectMaps().get(i);
			
			if (objectMap.hasObject(object))
				return objectMap;
		}
		return null;
	}
	
	/** input events management */
	public void mousePressed(MouseEvent e)
	{
		if (e.getButton() == MouseEvent.BUTTON3)
		{
			Segment obj = findObjAtMousePos();

			if (obj != null && obj.getType() == "object")
			{
				objectDragging = true;
				objectShape = (ObjShapeObject) obj;
				Undo.add(new UndoEditObject(findObjectMap(objectShape), objectShape));
				
				Point2D.Double real = new Point2D.Double(0, 0);
				screenToReal(e, real);
				objectOffset.setLocation(objectShape.getTrackLocation().x - real.x, objectShape.getTrackLocation().y - real.y);
			}
			else if (obj != null && obj.getType() == "relief")
			{
				reliefDragging = true;
				reliefShape = (ObjShapeRelief) obj;
				Undo.add(new UndoEditRelief(editorFrame.getReliefs().getReliefs(), reliefShape));

				Point2D.Double real = new Point2D.Double(0, 0);
				screenToReal(e, real);
				reliefPointIndex = reliefShape.getPointIndex(real);
				Point2D.Double point = reliefShape.getPoint2D(real);
				reliefOffset.setLocation(point.x - real.x, point.y - real.y);
			}
			else
			{
				Point2D.Double tmp = new Point2D.Double(imgOffsetStart.getX(),imgOffsetStart.getY());
				screenToReal(e, tmp);
				imgOffsetStart.setLocation(tmp.getX(),tmp.getY());
				imgOffsetPrev.setLocation(Editor.getProperties().getImgOffset());
			}
		}
		if (e.getButton() == MouseEvent.BUTTON1)
		{
			if (editorFrame.getTrackData() == null)
				return;

			try
			{
				screenToReal(e, clickPoint);

				// must check for a segment under the mouse
				Segment obj = findObjAtMousePos();

				Segment lastSelectedShape = selectedShape;

				boolean selectedShapeChanged = (selectedShape != obj);
				selectedShape = obj;

				handleDragging = -1;

				if (selectedShape != null)
				{
					dragging = true;

					int curHandle = 0;
					for (Iterator<Segment> i = handles.iterator(); i.hasNext(); curHandle++)
					{
						ObjShapeHandle h = (ObjShapeHandle) i.next();

						// is the mouse in the handledShape handle ?
						if (e.getX() > h.trPoints[0].getX() - ObjShapeHandle.handleSize
								&& e.getX() < h.trPoints[0].getX() + ObjShapeHandle.handleSize
								&& e.getY() > h.trPoints[0].getY() - ObjShapeHandle.handleSize
								&& e.getY() < h.trPoints[0].getY() + ObjShapeHandle.handleSize)
						{
							handleDragging = curHandle;
							selectedShape = handledShape;
							break;
						}
					}
				}
				else if (showObjects)
					noObjectSelected(e);

				if (lastSelectedShape != selectedShape)
					fireSelectionChanged(selectionChangedEvent);

				switch (currentState)
				{
					case STATE_NONE :
					{
						if (selectedShape != null)
						{
							if (selectedShape.getType().equals("object"))
							{
								objectSelected((ObjShapeObject)selectedShape, e);
							}
							else if (selectedShape.getType().equals("relief"))
							{
								reliefSelected((ObjShapeRelief)selectedShape, e);
							}
							else
							{
								Undo.add(new UndoSegmentChange(editorFrame, selectedShape));
								openSegmentDialog(selectedShape);
							}
						}

						if (selectedShapeChanged)
						{
							invalidate();
							repaint();
						}
					}
						break;

					case STATE_SHOW_BGRD_START_POSITION :
					{
						//						if (backgroundRectangle.contains(clickPoint))
						//						{
						//							backgroundRectangle.setRect(backgroundRectangle.getX()
						// - clickPoint.getX(),
						//									backgroundRectangle.getY() - clickPoint.getY(),
						// backgroundRectangle.getWidth(),
						//									backgroundRectangle.getHeight());
						//
						//							torcstuneSection.setAttributeOfPart("attnum", "name",
						// "bgrd img x", "val", Integer
						//									.toString((int) backgroundRectangle.getX()));
						//							torcstuneSection.setAttributeOfPart("attnum", "name",
						// "bgrd img y", "val", Integer
						//									.toString((int) backgroundRectangle.getY()));
						//							torcstuneSection.setAttributeOfPart("attnum", "name",
						// "bgrd img width", "val", Integer
						//									.toString((int) backgroundRectangle.getWidth()));
						//							torcstuneSection.setAttributeOfPart("attnum", "name",
						// "bgrd img height", "val", Integer
						//									.toString((int) backgroundRectangle.getHeight()));
						//
						//							editorFrame.documentIsModified = true;
						//							invalidate();
						//							repaint();
						//
						//							setState(STATE_NONE);
						//						}
					}
						break;
				}
			} catch (Exception ex)
			{
				ex.printStackTrace();
			}
		}
	}

	/** input events management */
	public void mouseReleased(MouseEvent e)
	{
		if (e.getButton() == MouseEvent.BUTTON1)
		{
			switch (currentState)
			{
				case STATE_NONE :
				{
					if (dragging)
					{
						try
						{
							this.redrawCircuit();
						} catch (Exception ex)
						{
							ex.printStackTrace();
						}
					}
				}
					break;
			}

			dragging = false;
		}
		else if (e.getButton() == MouseEvent.BUTTON3 && objectDragging)
		{
			objectDragging = false;
			objectShape = null;
		}
		else if (e.getButton() == MouseEvent.BUTTON3 && reliefDragging)
		{
			reliefDragging = false;
			reliefShape = null;
			reliefPointIndex = -1;
		}
	}

	/** input events management */
	public void mouseEntered(MouseEvent e)
	{
	}
	/** input events management */
	public void mouseExited(MouseEvent e)
	{
	}

	/** input events management */
	public void mouseDragged(MouseEvent e)
	{
		if (e.getModifiers() == 4)
		{
			if (objectShape != null && objectDragging == true)
			{
				Point2D.Double real = new Point2D.Double();
				screenToReal(e, real);
				
				real.x = real.x + objectOffset.x;
				real.y = real.y + objectOffset.y;
							
				ObjectMap objectMap = editorFrame.getObjectMaps().get(editorFrame.getCurrentObjectMap());

				int imageXY[] = { 0, 0 };
				realToImage(real, objectMap.getImageWidth(), objectMap.getImageHeight(), imageXY);
				
				objectShape.setImageXY(imageXY[0], imageXY[1]);				
				objectShape.calcShape(new Point2D.Double(real.x, real.y));
				
				editorFrame.documentIsModified = true;
				objectMap.setChanged(true);
				invalidate();
				repaint();
			}
			else if (reliefShape != null && reliefDragging == true)
			{
				Point2D.Double real = new Point2D.Double();
				screenToReal(e, real);
				
				real.x = real.x + reliefOffset.x;
				real.y = real.y + reliefOffset.y;

				reliefShape.setPoint2D(reliefPointIndex, real, boundingRectangle);
				editorFrame.documentIsModified = true;
				editorFrame.getReliefs().setChanged(true);
				invalidate();
				repaint();
			}
			else
			{
			    EditorPoint offset = Editor.getProperties().getImgOffset();
				Point2D.Double tmp = new Point2D.Double(0, 0);
				screenToReal(e, tmp);

				int x = (int) (imgOffsetPrev.getX() + (tmp.getX() - imgOffsetStart.getX()));
				int y = (int) (imgOffsetPrev.getY() + (tmp.getY() - imgOffsetStart.getY()));
				 
				offset.setLocation(x, y);
				Editor.getProperties().setImgOffset(offset);

				revalidate();
				invalidate();
				repaint();
			}	
		}
		else
		{
			screenToReal(e, mousePoint);

			switch (currentState)
			{
				case STATE_MOVE_SEGMENTS :
				{
					if (handleDragging == -1)
						return;

					if (handledShape.getType().equals("str"))
					{
						//						dragStraightEnd();
					} else
					{
						//						dragCurveEnd();
					}
				}
					break;
			}
		}
	}

	/** input events management */
	public void mouseMoved(MouseEvent e)
	{
		screenToReal(e, mousePoint);

		if (editorFrame.getCursorCoordinates())
		{
			invalidate();
			repaint();
		}

		try
		{
			switch (currentState)
			{
				case STATE_NONE :
				{
				}
					break;

				case STATE_CREATE_LEFT_SEGMENT :
				case STATE_CREATE_RIGHT_SEGMENT :
				case STATE_CREATE_STRAIGHT :
				case STATE_MOVE_SEGMENTS :
				case STATE_DELETE :
				case STATE_SUBDIVIDE :
				case STATE_FINISH_LINE :
				{
					if (dragging)
						return;

					// must check for a segment under the mouse
					Segment obj = findObjAtMousePos();

					if (obj == null)
					{
						if (handles.size() > 0)
						{
							handles.clear();
						}

						if (handledShape != null)
						{
							handledShape = null;

							invalidate();
							repaint();
						}
					}
					else
					{
						handles.clear();
						handledShape = obj;

						if (currentState == STATE_CREATE_LEFT_SEGMENT ||
							currentState == STATE_CREATE_RIGHT_SEGMENT ||
							currentState == STATE_CREATE_STRAIGHT ||
							currentState == STATE_FINISH_LINE)
						{
							double maxDist = Double.MAX_VALUE;
							Iterator<Segment> i = editorFrame.getTrackData().getSegments().iterator();
							while (i.hasNext())
							{
								Segment o = i.next();

								if (maxDist > o.endTrackCenter.distance(mousePoint))
								{
									maxDist = o.endTrackCenter.distance(mousePoint);
									obj = o;
								}
							}

							handledShape = obj;
							handle.calcShape(handledShape.endTrackCenter);

							handles.add(handle);
						}
						invalidate();
						repaint();
					}
				}
					break;

//				case STATE_DELETE :
//				{
//				}
//					break;
			}
		} catch (Exception ex)
		{
			ex.printStackTrace();
		}
	}

	public void mouseWheelMoved(MouseWheelEvent e)
	{
		screenToReal(e, mousePoint);

		getParent().dispatchEvent(e);

		if (editorFrame.getCursorCoordinates())
		{
			invalidate();
			repaint();
		}
	}

	//	/** input events management */
	//	/*
	//	 * public void mouseWheelMoved( MouseWheelEvent e ) { int n =
	//	 * e.getWheelRotation();
	//	 *
	//	 * for ( int i = 0; i < Math.abs( n ); i++ ) { if ( n < 0 )
	// incZoomFactor();
	//	 * else decZoomFactor(); } }
	//	 */
	//
	//	/**
	//	 * Drags end of handledShape (straight segment)
	//	 *
	//	 * @param currentPoint
	//	 * Point to move end of handledShape to (in meters)
	//	 */
	//	void dragStraightEnd()
	//	{
	//		try
	//		{
	//			// must find the curve segment concerned by this movement
	//
	//			XmlObjShape prevSegment = handledShape;
	//
	//			for (;;)
	//			{
	//				if (prevSegment == paintTrackSegments.get(0))
	//					return;
	//
	//				prevSegment = prevSegment.previousShape;
	//
	//				if (prevSegment == handledShape)
	//					return;
	//
	//				if (prevSegment.getClass().getName().indexOf("Curve") != -1)
	//					break;
	//			}
	//
	//			Curve curveSegment = (Curve) prevSegment;
	//
	//			if (curveSegment.allDatas[Curve.radiusStart] !=
	// curveSegment.allDatas[Curve.endRadius])
	//				return; // this curve segment doesn't have a constant radiusStart
	//
	//			XmlObjShape straightSegment = handledShape;
	//
	//			// define points O, B, C;
	//			Point2D.Double O, B, C;
	//
	//			O = curveSegment.center;
	//			B = mousePoint;
	//			C = curveSegment.startTrackCenter;
	//
	//			if (O.distance(B) == 0)
	//				return;
	//
	//			// calc A point (2 solutions)
	//			double alpha = Math.acos(Curve.radiusStart / O.distance(B));
	//			while (alpha < -Math.PI)
	//				alpha += EPMath.PI_MUL_2;
	//			while (alpha > Math.PI)
	//				alpha -= EPMath.PI_MUL_2;
	//
	//			double beta = Math.atan2(B.getY() - O.getY(), B.getX() - O.getX());
	//			while (beta < -Math.PI)
	//				beta += EPMath.PI_MUL_2;
	//			while (beta > Math.PI)
	//				beta -= EPMath.PI_MUL_2;
	//
	//			Point2D.Double A1 = new Point2D.Double();
	//			Point2D.Double A2 = new Point2D.Double();
	//
	//			double alphaToA1 = beta + alpha;
	//			double alphaToA2 = beta - alpha;
	//
	//			A1.setLocation(O.getX() + Math.cos(alphaToA1) * Curve.radiusStart,
	// O.getY() +
	// Math.sin(alphaToA1)
	//					* Curve.radiusStart);
	//			A2.setLocation(O.getX() + Math.cos(alphaToA2) * Curve.radiusStart,
	// O.getY() +
	// Math.sin(alphaToA2)
	//					* Curve.radiusStart);
	//
	//			// select 'good' solution
	//
	//			double d1 = A1.distance(curveSegment.endTrackCenter);
	//			double d2 = A2.distance(curveSegment.endTrackCenter);
	//
	//			double arc;
	//
	//			if (d1 < d2)
	//				arc = Math.atan2(C.getY() - O.getY(), C.getX() - O.getX())
	//						- Math.atan2(A1.getY() - O.getY(), A1.getX() - O.getX());
	//			else
	//				arc = Math.atan2(C.getY() - O.getY(), C.getX() - O.getX())
	//						- Math.atan2(A2.getY() - O.getY(), A2.getX() - O.getX());
	//			while (arc < 0)
	//				arc += EPMath.PI_MUL_2;
	//			while (arc >= EPMath.PI_MUL_2)
	//				arc -= EPMath.PI_MUL_2;
	//
	//			if (currentMovedShape != curveSegment)
	//			{
	//				// install undo step
	//				UndoStep undoStep = new UndoStepModifyTrackSegment(curveSegment);
	//				undoSteps.add(undoStep);
	//				redoSteps.clear();
	//				undoStep.redo();
	//
	//				currentMovedShape = curveSegment;
	//			}
	//
	//			curveSegment.segment.setAttributeOfPart("attnum", "name", "arc", "unit",
	// "rad");
	//			curveSegment.segment.setAttributeOfPartDouble("attnum", "name", "arc",
	// "val", Math.abs(arc));
	//
	//			calcGeometricObjects();
	//
	//			editorFrame.documentIsModified = true;
	//		} catch (Exception e)
	//		{
	//			e.printStackTrace();
	//		}
	//	}
	//
	//	/**
	//	 * Drags end of handledShape (curve segment)
	//	 *
	//	 * @param currentPoint
	//	 * Point to move end of handledShape to (in meters)
	//	 */
	//	void dragCurveEnd()
	//	{
	//		try
	//		{
	//			Point2D.Double a = handledShape.startTrackCenter;
	//			Point2D.Double b = mousePoint;
	//
	//			double dAB = b.distance(a);
	//
	//			if (dAB == 0)
	//				return;
	//
	//			double alpha = handledShape.startTrackAlpha;
	//			while (alpha < -Math.PI)
	//				alpha += EPMath.PI_MUL_2;
	//			while (alpha > Math.PI)
	//				alpha -= EPMath.PI_MUL_2;
	//
	//			double beta = Math.atan2(b.getY() - a.getY(), b.getX() - a.getX());
	//			while (beta < -Math.PI)
	//				beta += EPMath.PI_MUL_2;
	//			while (beta > Math.PI)
	//				beta -= EPMath.PI_MUL_2;
	//
	//			double gamma = (beta - alpha);
	//			while (gamma < -Math.PI)
	//				gamma += EPMath.PI_MUL_2;
	//			while (gamma > Math.PI)
	//				gamma -= EPMath.PI_MUL_2;
	//
	//			if (gamma == 0)
	//				return;
	//
	//			double arc = Math.abs(2 * gamma);
	//
	//			double r = dAB / (2 * Math.sin(gamma));
	//
	//			boolean rgt = handledShape.segment.getAttributeOfPart("attstr", "name",
	// "type", "val").equals("rgt");
	//
	//			if (Math.abs(r) < trackWidth
	//					/ 2
	//					+ handledShape.allDatas[rgt ? XmlObjShape.rightBorderWidth :
	// XmlObjShape.leftBorderWidth]
	//					+ handledShape.allDatas[rgt ? Math.max(XmlObjShape.rightSideStartWidth,
	//							XmlObjShape.rightSideEndWidth) : Math.max(XmlObjShape.leftSideStartWidth,
	//							XmlObjShape.leftSideEndWidth)])
	//				return; // radiusStart would be too little
	//
	//			if (!((r < 0 && rgt) || (r > 0 && !rgt)))
	//				return; // the curvature would be inversed
	//
	//			if (arc >= 360)
	//				return; // the arc would be to big
	//
	//			if (arc * Math.abs(r) > 2000)
	//				return; // the segment would be too long
	//
	//			if (currentMovedShape != handledShape)
	//			{
	//				// install undo step
	//				UndoStep undoStep = new UndoStepModifyTrackSegment(handledShape);
	//				undoSteps.add(undoStep);
	//				redoSteps.clear();
	//				undoStep.redo();
	//
	//				currentMovedShape = handledShape;
	//			}
	//
	//			handledShape.segment.setAttributeOfPart("attnum", "name", "radiusStart",
	// "unit", "m");
	//			handledShape.segment.setAttributeOfPartDouble("attnum", "name",
	// "radiusStart",
	// "val", Math.abs(r));
	//			handledShape.segment.setAttributeOfPartDouble("attnum", "name", "end
	// radiusStart", "val", Math.abs(r)); // constant
	//			// rayon
	//
	//			handledShape.segment.setAttributeOfPart("attnum", "name", "arc", "unit",
	// "rad");
	//			handledShape.segment.setAttributeOfPartDouble("attnum", "name", "arc",
	// "val", arc);
	//
	//			calcGeometricObjects();
	//
	//			editorFrame.documentIsModified = true;
	//		} catch (Exception e)
	//		{
	//			e.printStackTrace();
	//		}
	//	}
	//
	/**
	 * Look for an object under mouse position
	 * 
	 * @param e
	 *            Event containing mouse position (mouse move event)
	 * @return Object under mouse pos, or null if none
	 */
	protected Segment findObjAtMousePos()
	{
		if (editorFrame.getTrackData() == null)
			return null;
		
		// must look for an object under the mouse
		int count = 0;
		try
		{
			Iterator<Segment> i = editorFrame.getTrackData().getSegments().iterator();
			while (i.hasNext())
			{
				Segment obj = i.next();

				if (Class.forName("utils.circuit.Segment").isAssignableFrom(obj.getClass())
						&& obj.contains(mousePoint))
				{
					// object found !
					currentCount = count;
					return obj;
				}
				count++;
			}
		} catch (Exception ex)
		{
			ex.printStackTrace();
		}

		if (showReliefs)
		{
			Vector<ObjShapeRelief> reliefs = editorFrame.getReliefs().getReliefs();

			for (int i = 0; i < reliefs.size(); i++)
			{
				ObjShapeRelief relief = reliefs.get(i);

				try
				{
					if (Class.forName("utils.circuit.ObjShapeRelief").isAssignableFrom(relief.getClass())
							&& relief.contains(mousePoint))
					{
						// relief found !
						return relief;
					}
				}
				catch (Exception ex)
				{
					ex.printStackTrace();
				}
			}
		}

		if (showObjects)
		{
			Vector<ObjectMap> objectMaps = editorFrame.getObjectMaps();

			for (int j = 0; j < objectMaps.size(); j++)
			{
				Vector<ObjShapeObject> objects = objectMaps.get(j).getObjects();

				for (int k = 0; k < objects.size(); k++)
				{
					ObjShapeObject object = objects.get(k);

					try
					{
						if (Class.forName("utils.circuit.Segment").isAssignableFrom(object.getClass())
								&& object.contains(mousePoint))
						{
							// object found !
							return object;
						}
					}
					catch (Exception ex)
					{
						ex.printStackTrace();
					}
				}
			}
		}
		return null;
	}

	//	/**
	//	 * Set zoom factor and
	//	 */
	//	public void setZoomFactorSeeAllTerrain()
	//	{
	//	}
	//
	/**
	 * Augments zoom factor by multiplying it by 1.2
	 */
	public void incZoomFactor()
	{
		setZoomFactor(zoomFactor * 1.2);
	}

	/**
	 * Lowers zoom factor by dividing it by 1.2
	 */
	public void decZoomFactor()
	{
		Rectangle r = getVisibleRect();
		Rectangle2D.Double visibleRect = new Rectangle2D.Double(r.getX() / zoomFactor - outZoneWidth
				+ boundingRectangle.getX(), r.getY() / zoomFactor - outZoneHeight + boundingRectangle.getY(), r
				.getWidth()
				/ zoomFactor, r.getHeight() / zoomFactor);

		//		if (boundingRectangle.getWidth() < visibleRect.getWidth()
		//				&& boundingRectangle.getHeight() < visibleRect.getHeight())
		//			return;

		setZoomFactor(zoomFactor / 1.2);
	}

	/**
	 * Define precise zoom factor
	 * 
	 * @param newZoomFactor
	 */
	public void setZoomFactor(double newZoomFactor)
	{
		try
		{
			if (boundingRectangle == null)
			{
				return;
			}

			// update internal zoom factor
			zoomFactor = newZoomFactor;

			// new visible part of screen in pixels
			Rectangle r = getVisibleRect();
			
			// out zone size in meters
			outZoneWidth = (r.getWidth() / 2) / zoomFactor;
			outZoneHeight = (r.getHeight() / 2) / zoomFactor;

			// new visible part of screen in meters
			Rectangle2D.Double visibleRect = new Rectangle2D.Double(r.getX() / zoomFactor + boundingRectangle.getX()
					- outZoneWidth, r.getY() / zoomFactor + boundingRectangle.getY() - outZoneHeight, r.getWidth()
					/ zoomFactor, r.getHeight() / zoomFactor);

			// set new screen size in pixels
			setMaximumSize(new Dimension(
					(int) (newZoomFactor * (boundingRectangle.getWidth() + visibleRect.getWidth())),
					(int) (newZoomFactor * (boundingRectangle.getHeight() + visibleRect.getHeight()))));
			setMinimumSize(getMaximumSize());
			setPreferredSize(getMaximumSize());
			setSize(getMaximumSize());

			// define affine transformation
			affineTransform.setToIdentity();
			affineTransform.translate((outZoneWidth + boundingRectangle.getWidth() / 2) * zoomFactor,
					(outZoneHeight + boundingRectangle.getHeight() / 2) * zoomFactor);
			affineTransform.scale(zoomFactor, -zoomFactor);
			affineTransform.translate(-boundingRectangle.getX() - boundingRectangle.getWidth() / 2, -boundingRectangle
					.getY()
					- boundingRectangle.getHeight() / 2);
			inverseAffineTransform = affineTransform.createInverse();

			// scroll to keep same screen center as previously
			scrollRectToVisible(new Rectangle((int) (zoomFactor
					* (screenCenter.getX() - boundingRectangle.getX() + outZoneWidth) - r.getWidth() / 2) + 1,
					(int) (zoomFactor * (screenCenter.getY() - boundingRectangle.getY() + outZoneHeight) - r
							.getHeight() / 2) + 1, (int) (r.getWidth()) - 2, (int) (r.getHeight()) - 2));
			//			scrollRectToVisible( new Rectangle( (int)( zoomFactor * (
			// screenCenter.getX() ) - r.getWidth() / 2 ) + 1,
			//												(int)( zoomFactor * ( screenCenter.getY() ) - r.getHeight() / 2 )
			// + 1,
			//												(int)( r.getWidth() ) - 2,
			//												(int)( r.getHeight() ) - 2 ) );

			// calculate and draw
			revalidate();
			invalidate();
			repaint();
		} catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	/**
	 * Paint all paintable objects
	 * 
	 * @param g
	 */
	public void paint(Graphics g)
	{
		graphics = g;

		// visible part of screen in pixels
		Rectangle r = getVisibleRect();

		if (editorFrame.getCursorCoordinates())
		{
			if (editorFrame.getTrackData() != null)
			{
				String coordinates = String.format("x: %.3f y: %.3f",  + mousePoint.x, + mousePoint.y);

				graphics.drawString(coordinates, r.x + 10, r.y + 20);
			}
		}

		if (editorFrame.getTrackData() != null && editorFrame.getTrackData().getSegments() != null)
		{
			if (boundingRectangle == null)
			{
				return;
			}
			
			// out zone size in meters
			outZoneWidth = (r.getWidth() / 2) / zoomFactor;
			outZoneHeight = (r.getHeight() / 2) / zoomFactor;

			Rectangle2D.Double visibleRect = new Rectangle2D.Double(r.getX() / zoomFactor + boundingRectangle.getX()
					- outZoneWidth, r.getY() / zoomFactor + boundingRectangle.getY() - outZoneHeight, r.getWidth()
					/ zoomFactor, r.getHeight() / zoomFactor);

			// visible part of screen center in meters
			screenCenter = new Point2D.Double(visibleRect.getX() + visibleRect.getWidth() / 2, visibleRect.getY()
					+ visibleRect.getHeight() / 2);

			// Paints background image

			if (showBackground && backgroundImg != null)
			{
				Segment firstObj = editorFrame.getTrackData().getSegments().get(0);
				Point2D.Double p1 = new Point2D.Double(0, 0);
				p1.setLocation(firstObj.points[0]);
				p1.setLocation(p1.getX() + Editor.getProperties().getImgOffset().getX(), p1.getY() + Editor.getProperties().getImgOffset().getY());
				Point2D.Double p2 = new Point2D.Double(backgroundRectangle.getWidth(), backgroundRectangle.getHeight());
				p2.setLocation(p2.getX(), p2.getY());

				p1 = (Point2D.Double) affineTransform.transform(p1, null);

				g.drawImage(backgroundImg.getImage(), (int) (p1.getX()), (int) (p1.getY()),
						(int) (p2.getX() * zoomFactor), (int) (p2.getY() * zoomFactor), null);
			}

			// paint the circuit
			//			Rectangle2D.Double br = boundingRectangle;
			//			g.drawRect((int)br.x,(int)br.y,(int)br.width,(int)br.height);

			Iterator<Segment> i = editorFrame.getTrackData().getSegments().iterator();
			while (i.hasNext())
			{
				Segment obj = i.next();

				if (obj != selectedShape)
				{
					obj.draw(g, affineTransform);
				}
			}
			//		}
			//
			//			if (pits != null)
			//				pits.draw(g, affineTransform);

			if (terrain != null && terrainBorderMustBeShown)
			{
				terrain.calcShape(editorFrame, boundingRectangle);
				terrain.draw(g, affineTransform);
			}

			if (selectedShape != null)
			{
				g.setColor(Color.red);
				selectedShape.draw(g, affineTransform);
			}
			
			if (handledShape != null)
			{
				g.setColor(Color.YELLOW);
				handledShape.draw(g, affineTransform);
			}

			g.setColor(Color.blue);
			i = handles.iterator();

			while (i.hasNext())
			{
				i.next().draw(g, affineTransform);
			}

			if (showObjects)
			{
				Vector<ObjectMap> objectMaps = editorFrame.getObjectMaps();

				for (int j = 0; j < objectMaps.size(); j++)
				{
					Vector<ObjShapeObject> objects = objectMaps.get(j).getObjects();

					for (int k = 0; k < objects.size(); k++)
					{
						ObjShapeObject object = objects.get(k);

						g.setColor(object.getColor());
						if (object == selectedShape)
						{
							Graphics2D g2 = (Graphics2D) g;
			                g2.setStroke(new BasicStroke(3));
						}
						
						object.draw(g, affineTransform);
						
						if (object == selectedShape)
						{
							Graphics2D g2 = (Graphics2D) g;
			                g2.setStroke(new BasicStroke(1));
						}
					}
				}
			}

			if (showReliefs)
			{
				g.setColor(Color.blue);
				Reliefs reliefs = editorFrame.getReliefs();
				for (int j = 0; j < reliefs.getReliefs().size(); j++)
				{
					ObjShapeRelief relief = reliefs.getReliefs().get(j);

					relief.draw(g, affineTransform);
				}
			}
		}
	}

//	public Dimension getPreferredScrollableViewportSize()
//	{
//		return (getPreferredSize());
//	}
//	public int getScrollableUnitIncrement(Rectangle visibleRect, int orientation, int direction)
//	{
//		return (10);
//	}
//	public int getScrollableBlockIncrement(Rectangle visibleRect, int orientation, int direction)
//	{
//		return (10);
//	}
//	public boolean getScrollableTracksViewportWidth()
//	{
//		return (false);
//	}
//	public boolean getScrollableTracksViewportHeight()
//	{
//		return (false);
//	}


	private void jbInit() throws Exception
	{
		this.setMinimumSize(new Dimension(32767, 32767));
		this.setPreferredSize(new Dimension(32767, 32767));
		this.setSize(new Dimension(32767, 32767));

		//		this.addHierarchyBoundsListener(new
		// java.awt.event.HierarchyBoundsAdapter()
		//		{
		//			public void ancestorResized(HierarchyEvent e)
		//			{
		//				this_ancestorResized(e);
		//			}
		//		});
	}
	//
	//	void this_ancestorResized(HierarchyEvent e)
	//	{
	//		if (isAncestorResizing)
	//			return;
	//
	//		isAncestorResizing = true;
	//
	//		try
	//		{
	//			if (circuit != null)
	//			{
	//				// create the geometric representation of the circuit
	//				calcGeometricObjects();
	//			}
	//		} catch (Exception ex)
	//		{
	//			ex.printStackTrace();
	//		}
	//
	//		isAncestorResizing = false;
	//	}
	//
	public void setState(int state)
	{
		currentState = state;

		boolean mustFire = (selectedShape != null);
		selectedShape = null;

		if (mustFire)
			fireSelectionChanged(selectionChangedEvent);

		if (segmentParamDialog != null)
		{
			if (!segmentParamDialog.dirty)
			{
				//popUndo();
			}

			segmentParamDialog.dispose();
			segmentParamDialog = null;
		}
	}
	//
	//	public void showArrows(boolean show)
	//	{
	//		showArrows = show;
	//
	//		try
	//		{
	//			calcGeometricObjects();
	//		} catch (Exception e)
	//		{
	//			e.printStackTrace();
	//		}
	//	}
	//
//	public void popUndo()
//	{
//		//undoSteps.remove(undoSteps.size() - 1);
//	}

//	public void undo()
//	{
//		if (undoSteps.size() > 0)
//		{
//			UndoStep undoStep = (UndoStep) undoSteps.remove(undoSteps.size() - 1);
//			redoSteps.add(undoStep);
//
//			undoStep.undo();
//
//			try
//			{
//				this.redrawCircuit();
//			} catch (Exception e)
//			{
//				e.printStackTrace();
//			}
//
//			if (undoSteps.size() == 0)
//				editorFrame.documentIsModified = false;
//		}
//	}
//
//	public void redo()
//	{
//		if (redoSteps.size() > 0)
//		{
//			UndoStep undoStep = (UndoStep) redoSteps.remove(redoSteps.size() - 1);
//			undoSteps.add(undoStep);
//
//			undoStep.redo();
//
//			try
//			{
//				this.redrawCircuit();
//			} catch (Exception e)
//			{
//				e.printStackTrace();
//			}
//
//			editorFrame.documentIsModified = true;
//		}
//	}

	public void setBackgroundImage(String fileName)
	{
		Path imagePath = Paths.get(fileName);
		Path trackPath = Paths.get(Editor.getProperties().getPath());

		// add path to track to image file name if none present
		if (imagePath.getParent() == null)
		{
			imagePath = Paths.get(trackPath.toString(), fileName);
		}

		File img = new File(imagePath.toString());

		if (img.exists())
		{
			backgroundImg = new ImageIcon(imagePath.toString());
			int width = backgroundImg.getIconWidth();
			int height = backgroundImg.getIconHeight();
			double scale = Editor.getProperties().getImageScale();
			backgroundRectangle.setFrame(0, 0, width*scale, height*scale);
			editorFrame.documentIsModified = true;

			invalidate();
			repaint();
		}
		else
		{
			JOptionPane.showMessageDialog(this, "Couldn't open : " + fileName, "Background Image", JOptionPane.ERROR_MESSAGE);
			backgroundImg = null;
		}
	}
	
	/**
	 * @return Returns the showBackground.
	 */
	public boolean isShowBackground()
	{
		return showBackground;
	}
	/**
	 * @param showBackground The showBackground to set.
	 */
	public void setShowBackground(boolean showBackground)
	{
		this.showBackground = showBackground;
		if(showBackground && Editor.getProperties().getImage() != null)
		{
			setBackgroundImage(Editor.getProperties().getImage());
		}
	}

	public void setObjects(Vector<ObjectMap> objectMaps)
	{
		double border = editorFrame.getTrackData().getGraphic().getTerrainGeneration().getBorderMargin();

		if (Double.isNaN(border))
		{
			border = 0;
		}

		for (int i = 0; i < objectMaps.size(); i++)
	 	{
			ObjectMap objectMap = objectMaps.get(i);
			Vector<ObjShapeObject> objects = objectMap.getObjects();

			for (int j = 0; j < objects.size(); j++)
			{
				ObjShapeObject object = objects.get(j);

				Point2D.Double location = new Point2D.Double();
				imageToReal(object.getImageX(), object.getImageY(), objectMap.getImageWidth(), objectMap.getImageHeight(), location);

				object.calcShape(location);
			}
	 	}
	}

	public void setReliefs(Reliefs reliefs)
	{
		for (int i = 0; i < reliefs.getReliefs().size(); i++)
	 	{
			ObjShapeRelief relief = reliefs.getReliefs().get(i);

			relief.calcShape(boundingRectangle);
	 	}
	}

	/**
	 * @return Returns the showObjects.
	 */
	public boolean isShowObjects()
	{
		return showObjects;
	}
	/**
	 * @param showObjects The showObjects to set.
	 */
	public void setShowObjects(boolean showObjects)
	{
		this.showObjects = showObjects;
		if (showObjects && editorFrame.getObjectMaps().size() > 0)
		{
			setObjects(editorFrame.getObjectMaps());
			editorFrame.setCurrentObjectMap(0);
		}
	}

	/**
	 * @return Returns the showReliefs.
	 */
	public boolean isShowReliefs()
	{
		return showReliefs;
	}
	/**
	 * @param showReliefs The showObjects to set.
	 */
	public void setShowReliefs(boolean showReliefs)
	{
		this.showReliefs = showReliefs;
		if (showReliefs && editorFrame.getReliefs().getReliefs().size() > 0)
		{
			setReliefs(editorFrame.getReliefs());
		}
	}

	public void showTerrainBorder(boolean terrainBorderMustBeShown)
	{
		this.terrainBorderMustBeShown = terrainBorderMustBeShown;
	}

	public synchronized void removeSelectionListener(CircuitViewSelectionListener l)
	{
		if (selectionListeners != null && selectionListeners.contains(l))
		{
			Vector<CircuitViewSelectionListener> v = (Vector<CircuitViewSelectionListener>) selectionListeners.clone();
			v.removeElement(l);
			selectionListeners = v;
		}
	}

	public synchronized void addSelectionListener(CircuitViewSelectionListener l)
	{
		Vector<CircuitViewSelectionListener> v = selectionListeners == null ? new Vector<CircuitViewSelectionListener>(2) : (Vector<CircuitViewSelectionListener>) selectionListeners.clone();
		if (!v.contains(l))
		{
			v.addElement(l);
			selectionListeners = v;
		}
	}

	protected void fireSelectionChanged(CircuitViewSelectionEvent e)
	{
		if (selectionListeners != null)
		{
			Vector<CircuitViewSelectionListener> listeners = selectionListeners;
			int count = listeners.size();
			for (int i = 0; i < count; i++)
			{
				listeners.elementAt(i).circuitViewSelectionChanged(e);
			}
		}
	}

	public void redrawCircuit()
	{
		if (editorFrame.getTrackData() == null)
			return;

		SegmentVector track = editorFrame.getTrackData().getSegments();
		
		if (track == null)
			return;
		
		int size = track.size();
		
		Editor.getProperties().setCurrentA(0);
		Editor.getProperties().setCurrentY(0);
		Editor.getProperties().setCurrentX(0);
		
		for(int i=0; i<size; i++)
		{
			Segment obj = track.get(i);
			obj.setCount(i+1);
			try
			{
				obj.calcShape(editorFrame);
			} catch (Exception e)
			{
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		
		// calculate the bounding rectangle
		boundingRectangle = null;

		for(int i=0; i<size; i++)
		{
			Rectangle2D.Double r = track.get(i).getBounds();

			if (boundingRectangle == null)
				boundingRectangle = r;
			else
				boundingRectangle.add(r);
		}

		if (showObjects)
		{
			setObjects(editorFrame.getObjectMaps());
		}

		if (showReliefs)
		{
			setReliefs(editorFrame.getReliefs());
		}

//		 zoom
		setZoomFactor(zoomFactor);
	}
	
	private void openSegmentDialog(Segment shape)
	{
		if (segmentParamDialog != null)
		{
			segmentParamDialog.setShape(shape);
			segmentParamDialog.setVisible(true);
		}

		if (segmentParamDialog == null)
		{
			segmentParamDialog = new SegmentEditorDlg(this, editorFrame, "", false, shape);
			segmentParamDialog.addWindowListener(this);
		}
	}

	private void reliefSelected(ObjShapeRelief shape, MouseEvent me)
	{
		class AddPointBeforeAction extends AbstractAction
		{
			public AddPointBeforeAction(String text, ImageIcon icon, String desc)
			{
				super(text, icon);
				putValue(SHORT_DESCRIPTION, desc);
			}
			public void actionPerformed(ActionEvent e)
			{
				Vector<ObjShapeRelief> reliefs = editorFrame.getReliefs().getReliefs();
				int index = reliefs.indexOf(shape);

				if (index != -1)
				{
					Undo.add(new UndoEditRelief(reliefs, shape));					
					reliefs.get(index).addPoint2D(mousePoint, true);				
					reliefs.get(index).calcShape(boundingRectangle);
					selectedShape = null;
					invalidate();
					repaint();
					editorFrame.documentIsModified = true;
					return;
				}
			}
		}

		AddPointBeforeAction addPointBeforeAction = new AddPointBeforeAction("Add Point Before", null, "Add relief point before this point.");

		class AddPointAfterAction extends AbstractAction
		{
			public AddPointAfterAction(String text, ImageIcon icon, String desc)
			{
				super(text, icon);
				putValue(SHORT_DESCRIPTION, desc);
			}
			public void actionPerformed(ActionEvent e)
			{
				Vector<ObjShapeRelief> reliefs = editorFrame.getReliefs().getReliefs();
				int index = reliefs.indexOf(shape);

				if (index != -1)
				{
					Undo.add(new UndoEditRelief(reliefs, shape));					
					reliefs.get(index).addPoint2D(mousePoint, false);				
					reliefs.get(index).calcShape(boundingRectangle);
					selectedShape = null;
					invalidate();
					repaint();
					editorFrame.documentIsModified = true;
					return;
				}
			}
		}

		AddPointAfterAction addPointAfterAction = new AddPointAfterAction("Add Point After", null, "Add relief point after this point.");

/*
		class EditPointAction extends AbstractAction
		{
			public EditPointAction(String text, ImageIcon icon, String desc)
			{
				super(text, icon);
				putValue(SHORT_DESCRIPTION, desc);
			}
			public void actionPerformed(ActionEvent e)
			{
				JOptionPane.showMessageDialog(editorFrame, "Not implemented yet!", "Edit Relief Point", JOptionPane.INFORMATION_MESSAGE);
				selectedShape = null;
				invalidate();
				repaint();
			}
		}

		EditPointAction editPointAction = new EditPointAction("Edit Relief Point", null, "Edit relief point.");

		class EditReliefAction extends AbstractAction
		{
			public EditReliefAction(String text, ImageIcon icon, String desc)
			{
				super(text, icon);
				putValue(SHORT_DESCRIPTION, desc);
			}
			public void actionPerformed(ActionEvent e)
			{
				JOptionPane.showMessageDialog(editorFrame, "Not implemented yet!", "Edit Relief", JOptionPane.INFORMATION_MESSAGE);
				selectedShape = null;
				invalidate();
				repaint();
			}
		}

		EditReliefAction editReliefAction = new EditReliefAction("Edit Relief", null, "Edit relief.");
*/
		class DeletePointAction extends AbstractAction
		{
			public DeletePointAction(String text, ImageIcon icon, String desc)
			{
				super(text, icon);
				putValue(SHORT_DESCRIPTION, desc);
			}
			public void actionPerformed(ActionEvent e)
			{
				Vector<ObjShapeRelief> reliefs = editorFrame.getReliefs().getReliefs();
				int index = reliefs.indexOf(shape);

				if (index != -1)
				{
					Undo.add(new UndoEditRelief(reliefs, shape));
					reliefs.get(index).deletePoint2D(mousePoint);
					reliefs.get(index).calcShape(boundingRectangle);
					selectedShape = null;
					invalidate();
					repaint();
					editorFrame.documentIsModified = true;
					return;
				}
			}
		}

		DeletePointAction deletePointAction = new DeletePointAction("Delete Relief Point", null, "Delete relief point.");

		class DeleteReliefAction extends AbstractAction
		{
			public DeleteReliefAction(String text, ImageIcon icon, String desc)
			{
				super(text, icon);
				putValue(SHORT_DESCRIPTION, desc);
			}
			public void actionPerformed(ActionEvent e)
			{
				Vector<ObjShapeRelief> reliefs = editorFrame.getReliefs().getReliefs();
				int index = reliefs.indexOf(shape);
					
				if (index != -1)
				{
					Undo.add(new UndoDeleteRelief(reliefs, shape));
					reliefs.remove(shape);
					selectedShape = null;
					invalidate();
					repaint();
					editorFrame.documentIsModified = true;
					return;
				}
			}
		}

		DeleteReliefAction deleteReliefAction = new DeleteReliefAction("Delete Relief", null, "Delete relief.");

		JPopupMenu menu = new JPopupMenu();

		Vector<ObjShapeRelief> reliefs = editorFrame.getReliefs().getReliefs();
		int reliefIndex = reliefs.indexOf(shape);

		if (reliefIndex != -1)
		{
			int pointIndex = reliefs.get(reliefIndex).getPointIndex(mousePoint);
			
			if (pointIndex != -1)
			{
				String info = "Relief " + (reliefIndex + 1) + " Point " + (pointIndex + 1);
				JMenuItem itemInfo = new JMenuItem(info);
				menu.add(itemInfo);
				menu.addSeparator();
			}
		}
		
//		JMenuItem itemEditPoint = new JMenuItem("Edit Point");
//		JMenuItem itemEditRelief = new JMenuItem("Edit Relief");
		JMenuItem itemAddPointBefore = new JMenuItem("Add Point Before");
		JMenuItem itemAddPointAfter = new JMenuItem("Add Relief After");
		JMenuItem itemDeletePoint = new JMenuItem("Delete Point");
		JMenuItem itemDeleteRelief = new JMenuItem("Delete Relief");

//	    itemEditPoint.setAction(editPointAction);
//	    itemEditRelief.setAction(editReliefAction);
	    itemAddPointBefore.setAction(addPointBeforeAction);
	    itemAddPointAfter.setAction(addPointAfterAction);
	    itemDeletePoint.setAction(deletePointAction);
	    itemDeleteRelief.setAction(deleteReliefAction);

//	    menu.add(itemEditPoint);
//	    menu.add(itemEditRelief);
	    menu.add(itemAddPointBefore);
	    menu.add(itemAddPointAfter);
	    menu.add(itemDeletePoint);
	    menu.add(itemDeleteRelief);

	    menu.addPopupMenuListener(new PopupMenuListener()
	    {
	        public void popupMenuCanceled(final PopupMenuEvent e)
	        {
	    		selectedShape = null;
	    		invalidate();
	    		repaint();
	        }
	        public void popupMenuWillBecomeInvisible(final PopupMenuEvent e)
	        {
	        }
	        public void popupMenuWillBecomeVisible(final PopupMenuEvent e)
	        {	        	
	        }
	    });

	    menu.show(me.getComponent(), me.getX(), me.getY());
	}

	private void objectSelected(ObjShapeObject shape, MouseEvent me)
	{
		class CopyAction extends AbstractAction
		{
			public CopyAction(String text, ImageIcon icon, String desc)
			{
				super(text, icon);
				putValue(SHORT_DESCRIPTION, desc);
			}
			public void actionPerformed(ActionEvent e)
			{
				editorFrame.setCurrentObjectMap(0);	// this is wrong
				editorFrame.setCurrentObjectColor(shape.getRGB());
				selectedShape = null;
				invalidate();
				repaint();
			}
		}

		CopyAction copyAction = new CopyAction("Copy Object", null, "Edit object.");

		class EditAction extends AbstractAction
		{
			public EditAction(String text, ImageIcon icon, String desc)
			{
				super(text, icon);
				putValue(SHORT_DESCRIPTION, desc);
			}
			public void actionPerformed(ActionEvent e)
			{
				Vector<ObjectMap> objectMaps = editorFrame.getObjectMaps();
				
				for (int i = 0; i < objectMaps.size(); i++)
				{
					ObjectMap	objectMap = objectMaps.get(i);
					Vector<ObjShapeObject>	objects = objectMap.getObjects();
					
					for (int j = 0; j < objects.size(); j++)
					{
						if (objects.elementAt(j) == shape)
						{
							Undo.add(new UndoEditObject(objectMap, shape));

							TrackObjectDialog editObjectDialog = new TrackObjectDialog(editorFrame, false, shape, me.getXOnScreen(), me.getYOnScreen());
							
							editObjectDialog.setModal(true);
							editObjectDialog.setVisible(true);
							
							if (editObjectDialog.isChanged())
								editorFrame.documentIsModified = true;
							
							selectedShape = null;
							invalidate();
							repaint();
							return;
						}
					}
				}
			}
		}

		EditAction editAction = new EditAction("Edit Object", null, "Edit object.");

		class EditAllAction extends AbstractAction
		{
			public EditAllAction(String text, ImageIcon icon, String desc)
			{
				super(text, icon);
				putValue(SHORT_DESCRIPTION, desc);
			}
			public void actionPerformed(ActionEvent e)
			{
				Vector<ObjectMap> objectMaps = editorFrame.getObjectMaps();
				
				for (int i = 0; i < objectMaps.size(); i++)
				{
					ObjectMap	objectMap = objectMaps.get(i);
					Vector<ObjShapeObject>	objects = objectMap.getObjects();
					
					for (int j = 0; j < objects.size(); j++)
					{
						if (objects.elementAt(j) == shape)
						{
							int originalRGB = shape.getRGB();	// save the original rgb
							TrackObjectDialog editObjectsDialog = new TrackObjectDialog(editorFrame, true, shape, me.getXOnScreen(), me.getYOnScreen());

							editObjectsDialog.setModal(true);
							editObjectsDialog.setVisible(true);

							if (editObjectsDialog.isChanged())
							{
								int newRGB = shape.getRGB();	// save the new rgb
								shape.setRGB(originalRGB);		// restore the original rgb

								Vector<ObjectMapObject> editedObjects = new Vector<ObjectMapObject>();

								// find all the objects with the original color
								for (int k = 0; k < objectMaps.size(); k++)
								{
									objectMap = objectMaps.get(k);
									
									for (int l = 0; l < objectMap.getObjects().size(); l++)
									{
										ObjShapeObject object = objectMap.getObjects().get(l);
										
										if (object.getRGB() == originalRGB)
										{
											// remember the object to change
											editedObjects.add(new ObjectMapObject(objectMap, object));
										}
									}
								}

								Undo.add(new UndoEditAllObjects(editedObjects));

								// change all the objects
								for (int k = 0; k < editedObjects.size(); k++)
								{
									editedObjects.get(k).object.setRGB(newRGB);	
								}

								editorFrame.documentIsModified = true;
							}
							
							selectedShape = null;
							invalidate();
							repaint();
							return;
						}
					}
				}				
			}
		}

		EditAllAction editAllAction = new EditAllAction("Edit All Objects", null, "Edit all objects of this type.");

		class DeleteAction extends AbstractAction
		{
			public DeleteAction(String text, ImageIcon icon, String desc)
			{
				super(text, icon);
				putValue(SHORT_DESCRIPTION, desc);
			}
			public void actionPerformed(ActionEvent e)
			{
				Vector<ObjectMap> objectMaps = editorFrame.getObjectMaps();
				
				for (int i = 0; i < objectMaps.size(); i++)
				{
					ObjectMap objectMap = objectMaps.get(i);
					int index = objectMap.getObjectIndex(shape);
					
					if (index != -1)
					{
						Undo.add(new UndoDeleteObject(objectMap, shape));
						objectMap.removeObject(shape);
						selectedShape = null;
						invalidate();
						repaint();
						editorFrame.documentIsModified = true;
						return;
					}
				}
			}
		}

		DeleteAction deleteAction = new DeleteAction("Delete Object", null, "Delete object.");
		
		class DeleteAllAction extends AbstractAction
		{
			public DeleteAllAction(String text, ImageIcon icon, String desc)
			{
				super(text, icon);
				putValue(SHORT_DESCRIPTION, desc);
			}
			public void actionPerformed(ActionEvent e)
			{
				int rgb = shape.getRGB();
				Vector<ObjectMapObject> deletedObjects = new Vector<ObjectMapObject>();
				Vector<ObjectMap> objectMaps = editorFrame.getObjectMaps();
				
				for (int i = 0; i < objectMaps.size(); i++)
				{
					ObjectMap objectMap = objectMaps.get(i);
					
					for (int j = objectMap.getObjects().size() - 1; j >= 0; j--)
					{
						ObjShapeObject object = objectMap.getObjects().get(j);
						
						if (object.getRGB() == rgb)
						{
							deletedObjects.add(new ObjectMapObject(objectMap, object));
						}
					}
				}
				
				Undo.add(new UndoDeleteAllObjects(deletedObjects));
				
				for (int i = 0; i < deletedObjects.size(); i++)
				{
					ObjectMapObject deletedObject = deletedObjects.get(i);
					
					deletedObject.objectMap.removeObject(deletedObject.object);	
				}
				
				selectedShape = null;
				editorFrame.documentIsModified = true;
				invalidate();
				repaint();
			}
		}

		DeleteAllAction deleteAllAction = new DeleteAllAction("Delete All Objects", null, "Delete all objects of this type.");

		class DeleteAdjacentAction extends AbstractAction
		{
			int 					rgb = shape.getRGB();
			int						x = shape.getImageX();
			int						y = shape.getImageY();
			Vector<ObjectMapObject>	deletedObjects = new Vector<ObjectMapObject>();

			public DeleteAdjacentAction(String text, ImageIcon icon, String desc)
			{
				super(text, icon);
				putValue(SHORT_DESCRIPTION, desc);
			}
			public boolean findDeletedObject(ObjShapeObject object)
			{
				for (ObjectMapObject deleted : deletedObjects)
				{
					if (deleted.object == object)
					{
						return true;
					}
				}
				return false;
			}
			public void findAdjacent(ObjShapeObject objectShape)
			{
				for (ObjectMap objectMap : editorFrame.getObjectMaps())
				{
					for (ObjShapeObject object : objectMap.getObjects())
					{
						if (rgb == object.getRGB())
						{
							int dx = Math.abs(objectShape.getImageX() - object.getImageX());
							int dy = Math.abs(objectShape.getImageY() - object.getImageY());

							if ((dx == 1 && dy == 0) || (dx == 1 && dy == 1) || (dx == 0 && dy == 1))
							{
								if (!(object.getImageX() == x && object.getImageY() == y))
								{
									if (!findDeletedObject(object))
									{
										deletedObjects.add(new ObjectMapObject(objectMap, object));
										findAdjacent(object);
									}
								}
							}
						}
					}
				}
			}
			public void actionPerformed(ActionEvent e)
			{
				findAdjacent(shape);

				Undo.add(new UndoDeleteAllObjects(deletedObjects));

				for (ObjectMapObject deletedObject : deletedObjects)
				{
					deletedObject.objectMap.removeObject(deletedObject.object);
				}

				selectedShape = null;
				editorFrame.documentIsModified = true;
				invalidate();
				repaint();
			}
		}

		DeleteAdjacentAction deleteAdjacentAction = new DeleteAdjacentAction("Delete Adjacent Objects", null, "Delete adjacent objects of this type.");

		class DeleteAllAdjacentAction extends AbstractAction
		{
			int 					rgb = shape.getRGB();
			int						x = shape.getImageX();
			int						y = shape.getImageY();
			Vector<ObjectMapObject>	deletedObjects = new Vector<ObjectMapObject>();

			public DeleteAllAdjacentAction(String text, ImageIcon icon, String desc)
			{
				super(text, icon);
				putValue(SHORT_DESCRIPTION, desc);
			}
			public boolean findDeletedObject(ObjShapeObject object)
			{
				for (ObjectMapObject deleted : deletedObjects)
				{
					if (deleted.object == object)
					{
						return true;
					}
				}
				return false;
			}
			public void findAdjacent(ObjShapeObject objectShape)
			{
				for (ObjectMap objectMap : editorFrame.getObjectMaps())
				{
					for (ObjShapeObject object : objectMap.getObjects())
					{
						if (rgb == object.getRGB())
						{
							int dx = Math.abs(objectShape.getImageX() - object.getImageX());
							int dy = Math.abs(objectShape.getImageY() - object.getImageY());

							if ((dx == 1 && dy == 0) || (dx == 1 && dy == 1) || (dx == 0 && dy == 1))
							{
								if (!(object.getImageX() == x && object.getImageY() == y))
								{
									if (!findDeletedObject(object))
									{
										deletedObjects.add(new ObjectMapObject(objectMap, object));
										findAdjacent(object);
									}
								}
							}
						}
					}
				}
			}
			public void findAllAdjacent(ObjShapeObject objectShape)
			{
				for (ObjectMap objectMap : editorFrame.getObjectMaps())
				{
					for (ObjShapeObject object : objectMap.getObjects())
					{
						if (rgb == object.getRGB())
						{
							findAdjacent(object);
						}
					}
				}
			}
			public void actionPerformed(ActionEvent e)
			{
				findAllAdjacent(shape);

				Undo.add(new UndoDeleteAllObjects(deletedObjects));

				for (ObjectMapObject deletedObject : deletedObjects)
				{
					deletedObject.objectMap.removeObject(deletedObject.object);
				}

				selectedShape = null;
				editorFrame.documentIsModified = true;
				invalidate();
				repaint();
			}
		}

		DeleteAllAdjacentAction deleteAllAdjacentAction = new DeleteAllAdjacentAction("Delete All Adjacent Objects", null, "Delete all adjacent objects of this type.");

		JPopupMenu menu = new JPopupMenu();
		JMenuItem itemCopy = new JMenuItem("Copy");
	    JMenuItem itemEdit = new JMenuItem("Edit");
	    JMenuItem itemEditAll = new JMenuItem("Edit All");
	    JMenuItem itemDelete = new JMenuItem("Delete");
	    JMenuItem itemDeleteAll = new JMenuItem("Delete All");
	    JMenuItem itemDeleteAdjacent = new JMenuItem("Delete Adjacent");
	    JMenuItem itemDeleteAllAdjacent = new JMenuItem("Delete All Adjacent");

	    itemCopy.setAction(copyAction);
	    itemEdit.setAction(editAction);
	    itemEditAll.setAction(editAllAction);
	    itemDelete.setAction(deleteAction);
	    itemDeleteAll.setAction(deleteAllAction);
	    itemDeleteAdjacent.setAction(deleteAdjacentAction);
	    itemDeleteAllAdjacent.setAction(deleteAllAdjacentAction);

	    menu.add(itemCopy);
	    menu.addSeparator();
	    menu.add(itemEdit);
	    menu.add(itemEditAll);
	    menu.addSeparator();
	    menu.add(itemDelete);
	    menu.add(itemDeleteAll);
	    menu.addSeparator();
	    menu.add(itemDeleteAdjacent);
	    menu.add(itemDeleteAllAdjacent);

	    menu.addPopupMenuListener(new PopupMenuListener()
	    {
	        public void popupMenuCanceled(final PopupMenuEvent e)
	        {
	    		selectedShape = null;
	    		invalidate();
	    		repaint();
	        }
	        public void popupMenuWillBecomeInvisible(final PopupMenuEvent e)
	        {
	        }
	        public void popupMenuWillBecomeVisible(final PopupMenuEvent e)
	        {	        	
	        }
	    });

	    menu.show(me.getComponent(), me.getX(), me.getY());
	}

	private void addNewObject(MouseEvent me)
	{
		Vector<ObjectMap> objectMaps = editorFrame.getObjectMaps();
		
		if (objectMaps.isEmpty())
		{
			String[] options = {"Create", "Cancel"};
			
			int option = JOptionPane.showOptionDialog(this, "No Object Map",
					"No Object Map", JOptionPane.DEFAULT_OPTION, JOptionPane.INFORMATION_MESSAGE, null, options, options[0]);
			
			if (option == 1)					
				return;
			
			if (Double.isNaN(editorFrame.getTerrainGeneration().getBorderMargin()))
			{
				JOptionPane.showMessageDialog(editorFrame, "Setting margin to 100 meters.", "No Border Margin", JOptionPane.INFORMATION_MESSAGE);
				editorFrame.getTerrainGeneration().setBorderMargin(100);
			}
			
			boolean write = true;
			
			Path filename = Paths.get(Editor.getProperties().getPath() + sep + "object-map1.png");
			
			if (filename.toFile().exists())
			{
				String[] options1 = {"Overwrite", "Use Existing", "Cancel"};
				
				int option1 = JOptionPane.showOptionDialog(this, filename.getFileName().toString() + " already exists!",
						"Create Object Map", JOptionPane.DEFAULT_OPTION, JOptionPane.INFORMATION_MESSAGE, null, options1, options1[0]);
				
				if (option1 == 1)
				{
					ObjectMap	objectMap = new ObjectMap();
					
					objectMap.setName("map 1");
					try
					{
						objectMap.setObjectMap(filename.getFileName().toString());
					
						editorFrame.getObjectMaps().add(objectMap);
					}
					catch (IOException ex)
					{
					}
					
					write = false;
				}
				else if (option1 == 2)
				{
					return;
				}
			}
			
			if (write)
			{
				int imageWidth = 1024;
				int imageHeight = (int)Math.round(1024 * (boundingRectangle.height / boundingRectangle.width));
				BufferedImage image = new BufferedImage(imageWidth, imageHeight, BufferedImage.TYPE_INT_RGB);
				Graphics2D graphic = image.createGraphics();
				graphic.setColor(new Color(0x00000000));
				graphic.fillRect(0, 0, imageWidth, imageHeight);
			
				graphic.dispose();
				
				try
				{
					ImageIO.write(image, "png", new File(filename.toString()));
					
					ObjectMap	objectMap = new ObjectMap();
					
					objectMap.setName("map 1");
					objectMap.setObjectMap(filename.getFileName().toString());
					
					editorFrame.getObjectMaps().add(objectMap);
				}
				catch (IOException ex)
				{			
				}
			}
			
			editorFrame.setCurrentObjectMap(0);
		}
		
		ObjectMap objectMap = editorFrame.getObjectMaps().get(editorFrame.getCurrentObjectMap());
		Point2D.Double real = new Point2D.Double();
		screenToReal(me, real);

		int imageXY[] = { 0, 0 };
		realToImage(real, objectMap.getImageWidth(), objectMap.getImageHeight(), imageXY);
		
		ObjShapeObject	object = new ObjShapeObject(editorFrame.getObjectColorName(editorFrame.getCurrentObjectColor()), editorFrame.getCurrentObjectColor(), imageXY[0], imageXY[1]);
		
		TrackObjectDialog	addObjectDialog = new TrackObjectDialog(editorFrame, object, me.getXOnScreen(), me.getYOnScreen());
		
		addObjectDialog.setModal(true);
		addObjectDialog.setVisible(true);
		
		if (addObjectDialog.isChanged())
		{
			objectMap.addObject(object);			
			Undo.add(new UndoAddObject(objectMap, object));
			editorFrame.setCurrentObjectColor(object.getRGB());
			editorFrame.documentIsModified = true;
			redrawCircuit();
		}
	}

	private void realToImage(Point2D.Double real, int imageWidth, int imageHeight, int imageXY[])
	{
		double border = editorFrame.getTrackData().getGraphic().getTerrainGeneration().getBorderMargin();
		Rectangle2D.Double rect = new Rectangle2D.Double(boundingRectangle.getMinX() - border,
				 										 boundingRectangle.getMinY() - border,
				 										 boundingRectangle.getWidth() + (border * 2),
				 										 boundingRectangle.getHeight() + (border * 2));
		imageXY[0] = (int)Math.round(imageWidth * ((real.x + -rect.x) / rect.width));
		imageXY[1] = (int)Math.round(imageHeight * (1 - (((real.y + -rect.y) / rect.height))));
	}

	public void imageToReal(int imageX, int imageY, int imageWidth, int imageHeight, Point2D.Double real)
	{
		double border = editorFrame.getTrackData().getGraphic().getTerrainGeneration().getBorderMargin();
		Rectangle2D.Double rect = new Rectangle2D.Double(boundingRectangle.getMinX() - border,
														 boundingRectangle.getMinY() - border,
														 boundingRectangle.getWidth() + (border * 2),
														 boundingRectangle.getHeight() + (border * 2));
		double widthScale = rect.getWidth() / imageWidth;
		double heightScale = rect.getHeight() / imageHeight;

		real.x = rect.getMinX() + (imageX * widthScale);
		real.y = rect.getMinY() + ((imageHeight - imageY) * heightScale);
	}

	private void noObjectSelected(MouseEvent me)
	{
		class AddNewAction extends AbstractAction
		{
			public AddNewAction(String text, ImageIcon icon, String desc)
			{
				super(text, icon);
				putValue(SHORT_DESCRIPTION, desc);
			}
			public void actionPerformed(ActionEvent e)
			{
				addNewObject(me);
			}
		}

		AddNewAction addNewAction = new AddNewAction("Add New Object", null, "Add a new object.");

		class PasteAction extends AbstractAction
		{
			public PasteAction(String text, ImageIcon icon, String desc)
			{
				super(text, icon);
				putValue(SHORT_DESCRIPTION, desc);
			}
			public void actionPerformed(ActionEvent e)
			{
				ObjectMap objectMap = editorFrame.getObjectMaps().get(editorFrame.getCurrentObjectMap());
				Point2D.Double real = new Point2D.Double();
				screenToReal(me, real);

				int imageXY[] = { 0, 0 };
				realToImage(real, objectMap.getImageWidth(), objectMap.getImageHeight(), imageXY);
						
				ObjShapeObject	object = new ObjShapeObject(editorFrame.getObjectColorName(editorFrame.getCurrentObjectColor()), editorFrame.getCurrentObjectColor(), imageXY[0], imageXY[1]);
				objectMap.addObject(object);
				Undo.add(new UndoAddObject(objectMap, object));
				
				editorFrame.documentIsModified = true;
				redrawCircuit();
			}
		}

		if (me.isControlDown() && editorFrame.getCurrentObjectMap() != -1 && editorFrame.getCurrentObjectColor() != 0)
		{
			ObjectMap objectMap = editorFrame.getObjectMaps().get(editorFrame.getCurrentObjectMap());
			Point2D.Double real = new Point2D.Double();
			screenToReal(me, real);

			int imageXY[] = { 0, 0 };
			realToImage(real, objectMap.getImageWidth(), objectMap.getImageHeight(), imageXY);
			
			ObjShapeObject	object = new ObjShapeObject(editorFrame.getObjectColorName(editorFrame.getCurrentObjectColor()), editorFrame.getCurrentObjectColor(), imageXY[0], imageXY[1]);
			
			objectMap.addObject(object);
			Undo.add(new UndoAddObject(objectMap, object));
			editorFrame.setCurrentObjectColor(object.getRGB());
			editorFrame.documentIsModified = true;
			redrawCircuit();
			return;
		}
		
		PasteAction pasteAction = new PasteAction("Paste Object", null, "Paste object.");

		JPopupMenu menu = new JPopupMenu();
		
	    JMenuItem item1 = new JMenuItem("AddNew");

	    item1.setAction(addNewAction);

	    menu.add(item1);

		if (editorFrame.getCurrentObjectMap() != -1 && editorFrame.getCurrentObjectColor() != 0)
		{
		    JMenuItem item2 = new JMenuItem("Paste");

		    item2.setAction(pasteAction);

		    menu.add(item2);
		}

		selectedShape = null;
		invalidate();
		repaint();

		menu.show(me.getComponent(), me.getX(), me.getY());
	}

	/* (non-Javadoc)
	 * @see java.awt.event.WindowListener#windowActivated(java.awt.event.WindowEvent)
	 */
	public void windowActivated(WindowEvent e)
	{
		// TODO Auto-generated method stub
		
	}

	/* (non-Javadoc)
	 * @see java.awt.event.WindowListener#windowClosed(java.awt.event.WindowEvent)
	 */
	public void windowClosed(WindowEvent e)
	{
		//System.out.println("JDialog is closing");
		this.selectedShape = null;
		this.redrawCircuit();
	}

	/* (non-Javadoc)
	 * @see java.awt.event.WindowListener#windowClosing(java.awt.event.WindowEvent)
	 */
	public void windowClosing(WindowEvent e)
	{
		// TODO Auto-generated method stub
		
	}

	/* (non-Javadoc)
	 * @see java.awt.event.WindowListener#windowDeactivated(java.awt.event.WindowEvent)
	 */
	public void windowDeactivated(WindowEvent e)
	{
		// TODO Auto-generated method stub
		
	}

	/* (non-Javadoc)
	 * @see java.awt.event.WindowListener#windowDeiconified(java.awt.event.WindowEvent)
	 */
	public void windowDeiconified(WindowEvent e)
	{
		// TODO Auto-generated method stub
		
	}

	/* (non-Javadoc)
	 * @see java.awt.event.WindowListener#windowIconified(java.awt.event.WindowEvent)
	 */
	public void windowIconified(WindowEvent e)
	{
		// TODO Auto-generated method stub
		
	}

	/* (non-Javadoc)
	 * @see java.awt.event.WindowListener#windowOpened(java.awt.event.WindowEvent)
	 */
	public void windowOpened(WindowEvent e)
	{
		// TODO Auto-generated method stub
		
	}
}
