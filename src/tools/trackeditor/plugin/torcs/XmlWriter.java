/*
 *   XmlWriter.java
 *   Created on 14 ��� 2005
 *
 *    The XmlWriter.java is part of TrackEditor-0.6.0.
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

import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Vector;

import org.jdom.Attribute;
import org.jdom.Comment;
import org.jdom.DocType;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.output.Format;

import utils.Editor;
import utils.TrackData;
import utils.circuit.Camera;
import utils.circuit.Curve;
import utils.circuit.EnvironmentMapping;
import utils.circuit.ObjectMap;
import utils.circuit.Segment;
import utils.circuit.SegmentSide;
import utils.circuit.Straight;
import utils.circuit.Surface;
import utils.circuit.TrackLight;
import utils.circuit.TrackObject;

/**
 * @author Charalampos Alexopoulos
 *
 * TODO To change the template for this generated type comment go to Window -
 * Preferences - Java - Code Style - Code Templates
 */
public class XmlWriter
{
	//private static Properties	properties	= Properties.getInstance();
	static Document					doc;
	private boolean				optimize	= true;
	static boolean job;
	private static String sep = System.getProperty("file.separator");

	public static void writeXml()
	{
		job = false;
		getXml();
		writeToFile();
	}

	/**
	 * @param segments
	 * @return
	 */
	private static void getXml()
	{
		Comment com;
		Element root = getRoot();

		DocType type = new DocType("params", "../../../src/libs/tgf/params.dtd");
		String entity = "<!--  general definitions for tracks  -->\n";
		entity += "<!ENTITY default-surfaces SYSTEM \"../../../data/tracks/surfaces.xml\">\n";
		entity += "<!ENTITY default-objects SYSTEM \"../../../data/tracks/objects.xml\">\n";
		type.setInternalSubset(entity);
		doc = new Document();
		com = new Comment(getCredit());
		doc.addContent(com);
		com = new Comment(getLicence());
		doc.addContent(com);
		doc.addContent(type);
		doc.setRootElement(root);
		root.addContent(getLights());
		root.addContent(getSurfaces());
		root.addContent(getObjects());
		root.addContent(getHeader());
		root.addContent(getLocal());
		root.addContent(getGraphic());
		root.addContent(getGrid());
		root.addContent(getTrack());
		root.addContent(getCameras());
	}
	/**
	 * @return
	 */
	private synchronized static Element getRoot()
	{
		Attribute name = new Attribute("name", "test");
		Attribute val = new Attribute("type", "param");
		Attribute mode = new Attribute("mode", "mw");

		Element root = new Element("params");
		root.setAttribute(name);
		root.setAttribute(val);
		root.setAttribute(mode);

		return root;
	}

	/**
	 * @return
	 */
	private synchronized static Element getTrack()
	{
		Attribute name = new Attribute("name", "Main Track");
		Element track = new Element("section");
		Comment com = null;
		track.setAttribute(name);
		addContent(track, "width", "m", Editor.getProperties().getMainTrack().getWidth());
		addContent(track, "profil steps length", "m", Editor.getProperties().getMainTrack().getProfilStepsLength());
		addContent(track, "surface", Editor.getProperties().getMainTrack().getSurface());
		addContent(track, "raceline widthscale", null, Editor.getProperties().getMainTrack().getRacelineWidthscale());
		addContent(track, "raceline int", null, Editor.getProperties().getMainTrack().getRacelineInt());
		addContent(track, "raceline ext", null, Editor.getProperties().getMainTrack().getRacelineExt());

		if (Editor.getProperties().getHeader().getVersion() == 3)
		{
			getSideV3(track, Editor.getProperties().getMainTrack().getLeft(), "l");
			getSideV3(track, Editor.getProperties().getMainTrack().getLeft(), "r");
			getPitsV3(track);
		}
		else
		{
			com = new Comment("Left part of track");
			track.addContent(com);
			if (Editor.getProperties().getMainTrack().getLeft().getHasSide())
				track.addContent(getSide(Editor.getProperties().getMainTrack().getLeft(), "Left"));
			if (Editor.getProperties().getMainTrack().getLeft().getHasBorder())
				track.addContent(getBorder(Editor.getProperties().getMainTrack().getLeft(), "Left"));
			if (Editor.getProperties().getMainTrack().getLeft().getHasBarrier())
				track.addContent(getBarrier(Editor.getProperties().getMainTrack().getLeft(), "Left"));
			com = new Comment("End of left part");
			track.addContent(com);
			com = new Comment("Right part of track");
			track.addContent(com);
			if (Editor.getProperties().getMainTrack().getRight().getHasSide())
				track.addContent(getSide(Editor.getProperties().getMainTrack().getRight(), "Right"));
			if (Editor.getProperties().getMainTrack().getRight().getHasBorder())
				track.addContent(getBorder(Editor.getProperties().getMainTrack().getRight(), "Right"));
			if (Editor.getProperties().getMainTrack().getRight().getHasBarrier())
				track.addContent(getBarrier(Editor.getProperties().getMainTrack().getRight(), "Right"));
			com = new Comment("End of right part");
			track.addContent(com);
			track.addContent(getPits());
		}
		track.addContent(getSegments());

		return track;
	}

	/**
	 * @return
	 */
	private synchronized static Element getPits()
	{
		Element pits = new Element("section");
		pits.setAttribute(new Attribute("name", "Pits"));

		addContent(pits, "pit style", null, Editor.getProperties().getMainTrack().getPits().getStyle());
		addContent(pits, "side", Editor.getProperties().getMainTrack().getPits().getSide());
		addContent(pits, "entry", Editor.getProperties().getMainTrack().getPits().getEntry());
		addContent(pits, "start", Editor.getProperties().getMainTrack().getPits().getStart());
		addContent(pits, "start buildings", Editor.getProperties().getMainTrack().getPits().getStartBuildings());
		addContent(pits, "stop buildings", Editor.getProperties().getMainTrack().getPits().getStopBuildings());
		addContent(pits, "max pits", null, Editor.getProperties().getMainTrack().getPits().getMaxPits());
		addContent(pits, "end", Editor.getProperties().getMainTrack().getPits().getEnd());
		addContent(pits, "exit", Editor.getProperties().getMainTrack().getPits().getExit());
		addContent(pits, "length", "m", Editor.getProperties().getMainTrack().getPits().getLength());
		addContent(pits, "width", "m", Editor.getProperties().getMainTrack().getPits().getWidth());
		addContent(pits, "pit indicator", null, Editor.getProperties().getMainTrack().getPits().getIndicator());
		addContent(pits, "speed limit", "m", Editor.getProperties().getMainTrack().getPits().getSpeedLimit());

		return pits;
	}

	/**
	 * @return
	 */
	private synchronized static void getPitsV3(Element pits)
	{
		addContent(pits, "pit type", null, Editor.getProperties().getMainTrack().getPits().getStyle());
		addContent(pits, "pit side", Editor.getProperties().getMainTrack().getPits().getSide());
		addContent(pits, "pit entry", Editor.getProperties().getMainTrack().getPits().getEntry());
		addContent(pits, "pit start", Editor.getProperties().getMainTrack().getPits().getStart());
		addContent(pits, "start buildings", Editor.getProperties().getMainTrack().getPits().getStartBuildings());
		addContent(pits, "stop buildings", Editor.getProperties().getMainTrack().getPits().getStopBuildings());
		addContent(pits, "pit end", Editor.getProperties().getMainTrack().getPits().getEnd());
		addContent(pits, "pit exit", Editor.getProperties().getMainTrack().getPits().getExit());
		addContent(pits, "pit length", "m", Editor.getProperties().getMainTrack().getPits().getLength());
		addContent(pits, "pit width", "m", Editor.getProperties().getMainTrack().getPits().getWidth());
		addContent(pits, "speed limit", "m", Editor.getProperties().getMainTrack().getPits().getSpeedLimit());
	}

	/**
	 * @return
	 */
	private synchronized static Element getSegments()
	{
		Vector<Segment> segments = TrackData.getTrackData();
		Segment prev = null;
		Attribute name = null;
		if (Editor.getProperties().getHeader().getVersion() == 3)
			name = new Attribute("name", "segments");
		else
			name = new Attribute("name", "Track Segments");
		Comment com = null;
		Element trackSegs = new Element("section");
		trackSegs.setAttribute(name);

		for (int i = 0; i < segments.size(); i++)
		{
			Segment shape = segments.get(i);
			shape.previousShape = prev;
			com = new Comment("******************************");
			trackSegs.addContent(com);
			com = new Comment("     Segment " + (i + 1) + "                ");
			trackSegs.addContent(com);
			com = new Comment("******************************");
			trackSegs.addContent(com);
			trackSegs.addContent(getSegment(shape));
			prev = shape;
		}

		return trackSegs;
	}

	/**
	 * @param shape
	 * @return
	 */
	private synchronized static Element getSegment(Segment shape)
	{
		Attribute name = null;
		Comment com = null;
		Element el = null;
		Element segment = new Element("section");
		name = new Attribute("name", shape.getName());
		segment.setAttribute(name);
		el = attstrElement("type", shape.getType());
		segment.addContent(el);
		if (shape.getType().equals("str"))
		{
			addContent(segment, "lg", "m", ((Straight) shape).getLength());
		} else
		{
			double arc = ((Curve) shape).getArc();
			arc = (arc * 180) / Math.PI;
			addContent(segment, "arc", "deg", arc);

			double radStart = ((Curve) shape).getRadiusStart();
			addContent(segment, "radius", "m", radStart);

			double radEnd = ((Curve) shape).getRadiusEnd();
			if (radStart != radEnd)
			{
				addContent(segment, "end radius", "m", radEnd);
			}
			addContent(segment, "marks", ((Curve) shape).getMarks());
		}
		if (!Double.isNaN(shape.getHeightStartLeft()) &&
			!Double.isNaN(shape.getHeightStartRight()) &&
			shape.getHeightStartLeft() == shape.getHeightStartRight())
		{
			addContent(segment, "z start", "m", shape.getHeightStartLeft());
		}
		else
		{
			addContent(segment, "z start left", "m", shape.getHeightStartLeft());
			addContent(segment, "z start right", "m", shape.getHeightStartRight());
		}
		if (!Double.isNaN(shape.getHeightEndLeft()) &&
			!Double.isNaN(shape.getHeightEndRight()) &&
			shape.getHeightEndLeft() == shape.getHeightEndRight())
		{
			addContent(segment, "z end", "m", shape.getHeightEndLeft());
		}
		else
		{
			addContent(segment, "z end left", "m", shape.getHeightEndLeft());
			addContent(segment, "z end right", "m", shape.getHeightEndRight());
		}
		addContent(segment, "grade", "%", shape.getGrade());
		addContent(segment, "banking start", "deg", shape.getBankingStart());
		addContent(segment, "banking end", "deg", shape.getBankingEnd());
		addContent(segment, "profil", shape.getProfil());
		addContent(segment, "profil steps", "m", shape.getProfilSteps());
		addContent(segment, "profil steps length", "m", shape.getProfilStepsLength());
		addContent(segment, "profil start tangent", "m", shape.getProfilStartTangent());
		addContent(segment, "profil end tangent", "%", shape.getProfilEndTangent());
		addContent(segment, "profil start tangent left", "m", shape.getProfilStartTangentLeft());
		addContent(segment, "profil end tangent left", "m", shape.getProfilEndTangentLeft());
		addContent(segment, "profil start tangent right", "m", shape.getProfilStartTangentRight());
		addContent(segment, "profil end tangent right", "m", shape.getProfilEndTangentRight());
		addContent(segment, "surface", shape.getSurface());
		com = new Comment("Left part of segment");
		segment.addContent(com);
		if (shape.getLeft().getHasSide())
			segment.addContent(getSide(shape.getLeft(), "Left"));
		if (shape.getLeft().getHasBorder())
			segment.addContent(getBorder(shape.getLeft(), "Left"));
		if (shape.getLeft().getHasBarrier())
			segment.addContent(getBarrier(shape.getLeft(), "Left"));
		com = new Comment("End of left part");
		segment.addContent(com);
		com = new Comment("Right part of segment");
		segment.addContent(com);
		if (shape.getRight().getHasSide())
			segment.addContent(getSide(shape.getRight(), "Right"));
		if (shape.getRight().getHasBorder())
		    segment.addContent(getBorder(shape.getRight(), "Right"));
		if (shape.getRight().getHasBarrier())
		    segment.addContent(getBarrier(shape.getRight(), "Right"));
		com = new Comment("End of right part");
		segment.addContent(com);

		return segment;
	}

	/**
	 * @param left
	 * @param string
	 * @return
	 */
	private synchronized static Element getBorder(SegmentSide part, String sPart)
	{
		Element side = new Element("section");
		side.setAttribute(new Attribute("name", sPart + " Border"));

		addContent(side, "width", "m", part.getBorderWidth());
		addContent(side, "height", "m", part.getBorderHeight());
		addContent(side, "surface", part.getBorderSurface());
		addContent(side, "style", part.getBorderStyle());

		return side;
	}

	/**
	 * @param left
	 * @param string
	 * @return
	 */
	private synchronized static Element getBarrier(SegmentSide part, String sPart)
	{
		Element side = new Element("section");
		side.setAttribute(new Attribute("name", sPart + " Barrier"));

		addContent(side, "width", "m", part.getBarrierWidth());
		addContent(side, "height", "m", part.getBarrierHeight());
		addContent(side, "surface", part.getBarrierSurface());
		addContent(side, "style", part.getBarrierStyle());

		return side;
	}

	/**
	 * @param left
	 * @return
	 */
	private synchronized static void getSideV3(Element side, SegmentSide part, String sPart)
	{
		if (!Double.isNaN(part.getSideStartWidth()) &&
			!Double.isNaN(part.getSideEndWidth()) &&
			part.getSideStartWidth() == part.getSideEndWidth())
		{
			addContent(side, sPart + "side width", "m", part.getSideStartWidth());
		}
		else
		{
			addContent(side, sPart + "side start width", "m", part.getSideStartWidth());
			addContent(side, sPart + "side end width", "m", part.getSideEndWidth());
		}
		addContent(side, sPart + "side surface", part.getSideSurface());
		addContent(side, sPart + "side type", part.getSideBankingType());

		addContent(side, sPart + "border start width", "m", part.getBorderWidth());
		addContent(side, sPart + "border surface", part.getBorderSurface());
		addContent(side, sPart + "border type", part.getBorderStyle());
	}

	/**
	 * @param left
	 * @return
	 */
	private synchronized static Element getSide(SegmentSide part, String sPart)
	{
		Element side = new Element("section");
		side.setAttribute(new Attribute("name", sPart + " Side"));

		if (!Double.isNaN(part.getSideStartWidth()) &&
			!Double.isNaN(part.getSideEndWidth()) &&
			part.getSideStartWidth() == part.getSideEndWidth())
		{
			addContent(side, "width", "m", part.getSideStartWidth());
		}
		else
		{
			addContent(side, "start width", "m", part.getSideStartWidth());
			addContent(side, "end width", "m", part.getSideEndWidth());
		}
		addContent(side, "surface", part.getSideSurface());
		addContent(side, "banking type", part.getSideBankingType());

		return side;
	}

	private synchronized static Element getSurfaces()
	{
		Element surfaces = new Element("section");
		surfaces.setAttribute(new Attribute("name", "Surfaces"));
		Element root = surfaces;

		if (Editor.getProperties().getHeader().getVersion() == 3)
		{
			surfaces = new Element("section");
			surfaces.setAttribute(new Attribute("name", "List"));
			root.addContent(surfaces);
		}

		surfaces.setText("&default-surfaces;");

		Vector<Surface> surfaceData = Editor.getProperties().getSurfaces();

		if (surfaceData == null)
			return surfaces;

		for (int i = 0; i < surfaceData.size(); i++)
		{
			Surface surface = surfaceData.get(i);

			Element el = new Element("section");
			el.setAttribute(new Attribute("name", surface.getName()));

			addContent(el, "color R1", null, surface.getColorR1());
			addContent(el, "color G1", null, surface.getColorG1());
			addContent(el, "color B1", null, surface.getColorB1());
			addContent(el, "color R2", null, surface.getColorR2());
			addContent(el, "color G2", null, surface.getColorG2());
			addContent(el, "color B2", null, surface.getColorB2());
			addContent(el, "texture name", surface.getTextureName());
			addContent(el, "texture type", surface.getTextureType());
			addContent(el, "texture size", null, surface.getTextureSize());
			addContent(el, "texture link with previous", surface.getTextureLinkWithPrevious());
			addContent(el, "texture start on boundary", surface.getTextureStartOnBoundary());
			addContent(el, "texture mipmap", "m", surface.getTextureMipMap());
			addContent(el, "friction", null, surface.getFriction());
			addContent(el, "rolling resistance", null, surface.getRollingResistance());
			addContent(el, "bump name", surface.getBumpName());
			addContent(el, "bump size", null, surface.getBumpSize());
			addContent(el, "roughness", null, surface.getRoughness());
			addContent(el, "roughness wavelength", null, surface.getRoughnessWavelength());
			addContent(el, "raceline name", surface.getRacelineName());
			addContent(el, "dammage", null, surface.getDammage());
			addContent(el, "rebound", null, surface.getRebound());

			surfaces.addContent(el);
		}

		return root;
	}

	private synchronized static Element getCameras()
	{
		Element cameras = new Element("section");
		cameras.setAttribute(new Attribute("name", "Cameras"));
		Element root = cameras;

		Vector<Camera> cameraData = Editor.getProperties().getCameras();

		if (cameraData == null)
			return cameras;

		if (Editor.getProperties().getHeader().getVersion() == 3)
		{
			cameras = new Element("section");
			cameras.setAttribute(new Attribute("name", "list"));
			root.addContent(cameras);
		}

		for (int i = 0; i < cameraData.size(); i++)
		{
			Camera camera = cameraData.get(i);

			Element el = new Element("section");
			el.setAttribute(new Attribute("name", camera.getName()));

			addContent(el, "segment", camera.getSegment());
			addContent(el, "to right", null, camera.getToRight());
			addContent(el, "to start", null, camera.getToStart());
			addContent(el, "height", null, camera.getHeight());
			addContent(el, "fov start", camera.getFovStart());
			addContent(el, "fov end", camera.getFovEnd());

			cameras.addContent(el);
		}

		return cameras;
	}

	private synchronized static Element getLights()
	{
		Element lights = new Element("section");
		lights.setAttribute(new Attribute("name", "Track Lights"));
		Element root = lights;

		Vector<TrackLight> lightData = Editor.getProperties().getTrackLights();

		if (lightData == null)
			return lights;

		if (Editor.getProperties().getHeader().getVersion() == 3)
		{
			lights = new Element("section");
			lights.setAttribute(new Attribute("name", "List"));
			root.addContent(lights);
		}

		for (int i = 0; i < lightData.size(); i++)
		{
			TrackLight light = lightData.get(i);

			Element el = new Element("section");
			el.setAttribute(new Attribute("name", light.getName()));

			addContent(el, "role", light.getRole());

			if (light.getTopLeft() != null)
			{
				Element corner = new Element("section");
				corner.setAttribute(new Attribute("name", "topleft"));

				addContent(corner, "x", null, light.getTopLeft().x);
				addContent(corner, "y", null, light.getTopLeft().y);
				addContent(corner, "z", null, light.getTopLeft().z);

				el.addContent(corner);
			}

			if (light.getBottomRight() != null)
			{
				Element corner = new Element("section");
				corner.setAttribute(new Attribute("name", "bottomright"));

				addContent(corner, "x", null, light.getBottomRight().x);
				addContent(corner, "y", null, light.getBottomRight().y);
				addContent(corner, "z", null, light.getBottomRight().z);

				el.addContent(corner);
			}

			addContent(el, "texture on", light.getTextureOn());
			addContent(el, "texture off", light.getTextureOff());
			addContent(el, "index", null, light.getIndex());
			addContent(el, "red", null, light.getRed());
			addContent(el, "green", null, light.getGreen());
			addContent(el, "blue", null, light.getBlue());

			lights.addContent(el);
		}

		return lights;
	}

	private synchronized static Element getObjects()
	{
		Element objects = new Element("section");
		objects.setAttribute(new Attribute("name", "Objects"));
		objects.setText("&default-objects;");

		Vector<TrackObject> objectData = Editor.getProperties().getObjects();

		if (objectData == null)
			return objects;

		for (int i = 0; i < objectData.size(); i++)
		{
			TrackObject object = objectData.get(i);

			Element el = new Element("section");
			el.setAttribute(new Attribute("name", object.getName()));

			addContent(el, "object", object.getObject());
			addHexContent(el, "color", null, object.getColor());
			addContent(el, "orientation type", object.getOrientationType());
			addContent(el, "orientation", null, object.getOrientation());
			addContent(el, "delta height",null,  object.getDeltaHeight());
			addContent(el, "delta vert", null, object.getDeltaVert());

			objects.addContent(el);
		}

		return objects;
	}

	private synchronized static String getCredit()
	{
		String tmp = "\n";
		tmp += "file                : " + Editor.getProperties().getHeader().getName() + ".xml\n";
		tmp += "auto generated      : by " + Editor.getProperties().title + "\n";
		tmp += "version             : " + Editor.getProperties().version + "\n";
		tmp += "copyright           : (C) 2005 by Charalampos Alexopoulos\n";
		tmp += "email               : \n";

		return tmp;
	}

	private synchronized static String getLicence()
	{
		String tmp = "\n";
		tmp += "This program is free software; you can redistribute it and/or modify it\n";
		tmp += "under the terms of the GNU General Public License as published by\n";
		tmp += "the Free Software Foundation; either version 2 of the License, or\n";
		tmp += "(at your option) any later version.";
		tmp += "\n";

		return tmp;
	}

	private synchronized static Element getHeader()
	{
		Attribute name = new Attribute("name", "Header");
		String tmp = "";

		Element header = new Element("section");
		header.setAttribute(name);

		addContent(header, "name", Editor.getProperties().getHeader().getName());

		if (Editor.getProperties().getHeader().getCategory() != null)
		{
			tmp = Editor.getProperties().getHeader().getCategory();
		} else
		{
			tmp = "road";
		}
		addContent(header, "category", tmp);
		addContent(header, "subcategory", Editor.getProperties().getHeader().getSubcategory());
		addContent(header, "version", null, Editor.getProperties().getHeader().getVersion());
		addContent(header, "sky version", null, Editor.getProperties().getHeader().getSkyVersion());

		if (Editor.getProperties().getHeader().getAuthor() != null)
		{
			tmp = Editor.getProperties().getHeader().getAuthor();
		} else
		{
			tmp = "Anonymous";
		}
		addContent(header, "author", tmp);

		if (Editor.getProperties().getHeader().getDescription() != null)
		{
			tmp = Editor.getProperties().getHeader().getDescription();
		} else
		{
			tmp = "No description provided";
		}
		addContent(header, "description", tmp);

		return header;
	}

	/**
	 * @return
	 */
	private synchronized static Element getLocal()
	{
		Element element = new Element("section");
		element.setAttribute(new Attribute("name", "Local Info"));

		addContent(element, "station", Editor.getProperties().getLocalInfo().getStation());
		addContent(element, "timezone", null, Editor.getProperties().getLocalInfo().getTimezone());
		addContent(element, "overall rain likelyhood", "%", Editor.getProperties().getLocalInfo().getOverallRainLikelyhood());
		addContent(element, "little rain likelyhood", "%", Editor.getProperties().getLocalInfo().getLittleRainLikelyhood());
		addContent(element, "medium rain likelyhood", "%", Editor.getProperties().getLocalInfo().getMediumRainLikelyhood());
		addContent(element, "time of day", "hour", Editor.getProperties().getLocalInfo().getTimeOfDay());
		addContent(element, "sun ascension", "deg", Editor.getProperties().getLocalInfo().getSunAscension());
		addContent(element, "altitude", "m", Editor.getProperties().getLocalInfo().getAltitude());

		return element;
	}

	private synchronized static Element getGrid()
	{
		Element element = new Element("section");
		element.setAttribute(new Attribute("name", "Starting Grid"));

		addContent(element, "rows", null, Editor.getProperties().getStartingGrid().getRows());
		addContent(element, "pole position side", Editor.getProperties().getStartingGrid().getPolePositionSide());
		addContent(element, "distance to start", "m", Editor.getProperties().getStartingGrid().getDistanceToStart());
		addContent(element, "distance between columns", "m", Editor.getProperties().getStartingGrid().getDistanceBetweenColumns());
		addContent(element, "offset within a column", "m", Editor.getProperties().getStartingGrid().getOffsetWithinAColumn());
		addContent(element, "initial height", "m", Editor.getProperties().getStartingGrid().getInitialHeight());

		return element;
	}

	/**
	 * @return
	 */
	private synchronized static Element getGraphic()
	{
		Element element = new Element("section");
		element.setAttribute(new Attribute("name", "Graphic"));

		addContent(element, "3d description", Editor.getProperties().getHeader().getName() + ".ac");	// TODO
		addContent(element, "3d description night", Editor.getProperties().getGraphic().getDescriptionNight());
		addContent(element, "3d description rain+night", Editor.getProperties().getGraphic().getDescriptionRainNight());
		addContent(element, "background image", Editor.getProperties().getGraphic().getBackgroundImage());
		addContent(element, "background type", null, Editor.getProperties().getGraphic().getBackgroundType());
		addContent(element, "background color R", null, Editor.getProperties().getGraphic().getBackgroundColorR());
		addContent(element, "background color G", null, Editor.getProperties().getGraphic().getBackgroundColorG());
		addContent(element, "background color B", null, Editor.getProperties().getGraphic().getBackgroundColorB());
		addContent(element, "ambient color R", null, Editor.getProperties().getGraphic().getAmbientColorR());
		addContent(element, "ambient color G", null, Editor.getProperties().getGraphic().getAmbientColorG());
		addContent(element, "ambient color B", null, Editor.getProperties().getGraphic().getAmbientColorB());
		addContent(element, "diffuse color R", null, Editor.getProperties().getGraphic().getDiffuseColorR());
		addContent(element, "diffuse color G", null, Editor.getProperties().getGraphic().getDiffuseColorG());
		addContent(element, "diffuse color B", null, Editor.getProperties().getGraphic().getDiffuseColorB());
		addContent(element, "specular color R", null, Editor.getProperties().getGraphic().getSpecularColorR());
		addContent(element, "specular color G", null, Editor.getProperties().getGraphic().getSpecularColorG());
		addContent(element, "specular color B", null, Editor.getProperties().getGraphic().getSpecularColorB());
		addContent(element, "light position x", null, Editor.getProperties().getGraphic().getLightPositionX());
		addContent(element, "light position y", null, Editor.getProperties().getGraphic().getLightPositionY());
		addContent(element, "light position z", null, Editor.getProperties().getGraphic().getLightPositionZ());
		addContent(element, "shininess", null, Editor.getProperties().getGraphic().getShininess());
		addContent(element, "fov factor", null, Editor.getProperties().getGraphic().getFovFactor());

		element.addContent(getTurnMarks());
		element.addContent(getTerrainGeneration());
		element.addContent(getEnvironmentMapping());

		return element;
	}

	private synchronized static Element getTurnMarks()
	{
		Element element = new Element("section");
		element.setAttribute(new Attribute("name", "Turn Marks"));

		addContent(element, "width", "m", Editor.getProperties().getGraphic().getTurnMarks().getWidth());
		addContent(element, "height", "m", Editor.getProperties().getGraphic().getTurnMarks().getHeight());
		addContent(element, "vertical space", "m", Editor.getProperties().getGraphic().getTurnMarks().getVerticalSpace());
		addContent(element, "horizontal space", "m", Editor.getProperties().getGraphic().getTurnMarks().getHorizontalSpace());

		return element;
	}

	private synchronized static Element getTerrainGeneration()
	{
		Element element = new Element("section");
		element.setAttribute(new Attribute("name", "Terrain Generation"));

		addContent(element, "track step", "m", Editor.getProperties().getGraphic().getTerrainGeneration().getTrackStep());
		addContent(element, "border margin", "m", Editor.getProperties().getGraphic().getTerrainGeneration().getBorderMargin());
		addContent(element, "border step", "m", Editor.getProperties().getGraphic().getTerrainGeneration().getBorderStep());
		addContent(element, "border height", "m", Editor.getProperties().getGraphic().getTerrainGeneration().getBorderHeight());
		addContent(element, "orientation", Editor.getProperties().getGraphic().getTerrainGeneration().getOrientation());
		addContent(element, "maximum altitude", "m", Editor.getProperties().getGraphic().getTerrainGeneration().getMaximumAltitude());
		addContent(element, "minimum altitude", "m", Editor.getProperties().getGraphic().getTerrainGeneration().getMinimumAltitude());
		addContent(element, "group size", "m", Editor.getProperties().getGraphic().getTerrainGeneration().getGroupSize());
		addContent(element, "elevation map", Editor.getProperties().getGraphic().getTerrainGeneration().getElevationMap());
		addContent(element, "relief file", Editor.getProperties().getGraphic().getTerrainGeneration().getReliefFile());
		addContent(element, "surface", Editor.getProperties().getGraphic().getTerrainGeneration().getSurface());

		element.addContent(getObjectMaps());

		return element;
	}

	private synchronized static Element getEnvironmentMapping()
	{
		Element element = new Element("section");
		element.setAttribute(new Attribute("name", "Environment Mapping"));

		Vector<EnvironmentMapping> environment = Editor.getProperties().getGraphic().getEnvironmentMapping();

		for (int i = 0; i < environment.size(); i++)
		{
			EnvironmentMapping data = environment.get(i);

			Element el = new Element("section");
			el.setAttribute(new Attribute("name", data.getName()));

			addContent(el, "env map image", data.getEnvMapImage());

			element.addContent(el);
		}

		return element;
	}

	private synchronized static Element getObjectMaps()
	{
		Element element = new Element("section");
		element.setAttribute(new Attribute("name", "Object Maps"));

		Vector<ObjectMap> objMaps = Editor.getProperties().getGraphic().getTerrainGeneration().getObjectMaps();

		for (int i = 0; i < objMaps.size(); i++)
		{
			ObjectMap data = objMaps.get(i);

			Element el = new Element("section");
			el.setAttribute(new Attribute("name", data.getName()));

			addContent(el, "object map", data.getObjectMap());

			element.addContent(el);
		}

		return element;
	}

	private synchronized static Element attstrElement(String attname, String attval)
	{
		Attribute name = null;
		Attribute val = null;

		Element el = new Element("attstr");
		name = new Attribute("name", attname);
		if (attval == null)
		{
			attval = "";
		}
		val = new Attribute("val", attval);
		el.setAttribute(name);
		el.setAttribute(val);

		return el;
	}

	private synchronized static Element attnumElement(String attname, String attunit, String attval)
	{
		Attribute name = null;
		Attribute unit = null;
		Attribute val = null;

		Element el = new Element("attnum");
		name = new Attribute("name", attname);
		el.setAttribute(name);
		if (attunit != null)
		{
			unit = new Attribute("unit", attunit);
			el.setAttribute(unit);
		}
		val = new Attribute("val", attval);
		el.setAttribute(val);

		return el;
	}

	private synchronized static void addContent(Element section, String attribute, String units, double value)
	{
		if (!Double.isNaN(value))
		{
			section.addContent(attnumElement(attribute, units, value + ""));
		}
	}

	private synchronized static void addContent(Element section, String attribute, String units, int value)
	{
		if (value != Integer.MAX_VALUE)
		{
			section.addContent(attnumElement(attribute, units, value + ""));
		}
	}

	private synchronized static void addHexContent(Element section, String attribute, String units, int value)
	{
		if (value != Integer.MAX_VALUE)
		{
			section.addContent(attnumElement(attribute, units, "0x"+Integer.toHexString(value).toUpperCase()));
		}
	}

	private synchronized static void addContent(Element section, String attribute, String string)
	{
		if (string != null && !string.isEmpty())
		{
			section.addContent(attstrElement(attribute, string));
		}
	}

	private synchronized static void writeToFile()
	{
		String fileName = Editor.getProperties().getPath() + sep + Editor.getProperties().getHeader().getName() + ".xml";
		try
		{
			FileOutputStream out = new FileOutputStream(fileName);
			XMLOutput op = new XMLOutput(Format.getPrettyFormat());
			op.output(doc, out);
			out.flush();
			out.close();
		} catch (IOException e)
		{
			System.err.println(e);
		}
	}
}