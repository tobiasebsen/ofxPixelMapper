/*
 *  ofxPixelMapper.cpp
 *
 *  Created by Tobias Ebsen on 6/7/13.
 *
 */

#include "ofxPixelMapper.h"


void ofxPixelMapper::setup(int width, int height) {
	this->width = width;
	this->height = height;
}

void ofxPixelMapper::allocate(int num, int universes) {
	
	this->numAreas = num;
	this->numUniverses = universes;
	
	texCoords.allocate(num * 4, 2, 2, GL_STATIC_DRAW);
	vertices.allocate(num * 4, 2, 2, GL_STATIC_DRAW);
	pixels.allocate(universes * 256, 3, 3, GL_DYNAMIC_COPY);
	fboPixels.allocate(256, universes, GL_RGB);
	
	colorPos.allocate(num * 4, 2, 2, GL_STATIC_DRAW);
	colors.allocate(num * 4, 3, 3, GL_DYNAMIC_COPY);
	fboColors.allocate(256, num / 256 + 1, GL_RGB);
	
	fboColors.bind();
	ofClear(0);
	fboColors.unbind();
	
	int *pData = colorPos.map();
	int h = num / 256 + 1;
	for (int y=0; y<h; y++) {
		for (int x=0; x<256; x++) {
			pData[0] = x;
			pData[1] = y;
			pData[2] = x + 1;
			pData[3] = y;
			pData[4] = x + 1;
			pData[5] = y + 1;
			pData[6] = x;
			pData[7] = y + 1;
			pData += 8;
		}
	}
	colorPos.unmap();
}

void ofxPixelMapper::updatePixels(ofTexture & tex) {
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);

	fboPixels.begin();
	tex.bind();
	texCoords.begin();
	
	vertices.draw(GL_QUADS);
	
	texCoords.end();
	tex.unbind();
	
	fboPixels.end();
	
	pixels.readPixels(fboPixels);
}

void ofxPixelMapper::copyPixels(int universe, unsigned char* data, int length) {

	unsigned char* pData = pixels.map();
	memcpy(data, pData + universe * 256 * 3, length);
	pixels.unmap();
}

void ofxPixelMapper::drawColors(ofTexture & tex) {
		
	fboColors.begin();
	tex.bind();
	texCoords.begin();
	colorPos.draw(GL_QUADS);
	texCoords.end();
	tex.unbind();
	fboColors.end();
	
	colors.readPixels(fboColors);
	
	unsigned char *cData = colors.map();
	colors.unmap();
	
	texCoords.bind(GL_ARRAY_BUFFER);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, NULL);

	for (int i=0; i<numAreas; i++) {
	
		glColor3ub(cData[0], cData[1], cData[2]);
		//glColor3ub(255, 0, 0);
		glDrawArrays(GL_QUADS, i * 4, 4);
		cData += 3;
	}
	
	glDisableClientState(GL_VERTEX_ARRAY);
	texCoords.unbind();

		
	glColor3ub(255, 255, 255);
}

void ofxPixelMapper::drawAreas(int mode) {
	
	if (mode == GL_LINES) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		mode = GL_QUADS;
	}
	
	texCoords.draw(mode, 0, numAreas * 4);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void ofxPixelMapper::drawArea(int index, int mode) {
	
	if (mode == GL_LINES) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		mode = GL_QUADS;
	}
	
	texCoords.draw(mode, index * 4, 4);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void ofxPixelMapper::setArea(int index, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {

	float *tData = texCoords.map() + index * 8;
	tData[0] = x1;
	tData[1] = y1;
	tData[2] = x2;
	tData[3] = y2;
	tData[4] = x3;
	tData[5] = y3;
	tData[6] = x4;
	tData[7] = y4;
	texCoords.unmap();
}

void ofxPixelMapper::setArea(int index, ofPoint p1, ofPoint p2, ofPoint p3, ofPoint p4) {
	setArea(index, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y);
}

void ofxPixelMapper::getArea(int index, ofPoint& p1, ofPoint& p2, ofPoint& p3, ofPoint& p4) {

	float *tData = texCoords.map() + index * 8;
	p1.x = tData[0];
	p1.y = tData[1];
	p2.x = tData[2];
	p2.y = tData[3];
	p3.x = tData[4];
	p3.y = tData[5];
	p4.x = tData[6];
	p4.y = tData[7];
	texCoords.unmap();
}

ofPoint ofxPixelMapper::getAreaCenter(int index) {
	
	ofPoint center;
	float *tData = texCoords.map() + index * 8;
	for (int i=0; i<4; i++) {
		center.x += tData[0];
		center.y += tData[1];
		tData += 2;
	}
	texCoords.unmap();
	return center / 4;
}

int ofxPixelMapper::findArea(float x, float y) {
	
	mapArea *areas = (mapArea*)texCoords.map();
	if (areas == NULL)
		return -1;

	ofPoint point(x, y);
	int index = -1;
	
	for (int i=0; i<numAreas; i++) {
		if (pointInArea(point, &areas[i])) {
			index = i;
			break;
		}
	}
	texCoords.unmap();
	
	return index;
}

int ofxPixelMapper::findByDmx(int index, float threshold, int dmx) {
	
	ofPoint center = getAreaCenter(index);
	float dist = threshold;
	int next = -1;
	
	for (int i=0; i<numAreas; i++) {
		if (i != index) {
			ofPoint c = getAreaCenter(i);
			if (center.distance(c) < dist) {
				dist = center.distance(c);
				if (getDmx(i) == dmx) {
					next = i;
				}
			}
		}
	}
	return next;
}

int ofxPixelMapper::findByUniverse(int index, float threshold, int universe) {
	
	ofPoint center = getAreaCenter(index);
	float dist = threshold;
	int next = -1;
	
	for (int i=0; i<numAreas; i++) {
		if (i != index) {
			ofPoint c = getAreaCenter(i);
			if (center.distance(c) < dist) {
				dist = center.distance(c);
				if (getUniverse(i) == universe) {
					next = i;
				}
			}
		}
	}
	return next;
}

void ofxPixelMapper::setSize(int width, int height) {
	this->width = width;
	this->height = height;
}

int ofxPixelMapper::getWidth() {
	return width;
}

int ofxPixelMapper::getHeight() {
	return height;
}

int ofxPixelMapper::getNumAreas() {
	return numAreas;
}

int ofxPixelMapper::getNumUniverses() {
	return numUniverses;
}

int ofxPixelMapper::getUniverse(int index) {
	int universe = vertices.map()[index * 8 + 1];
	vertices.unmap();
	return universe;
}

int ofxPixelMapper::getDmx(int index) {
	int dmx = vertices.map()[index * 8] * 3;
	vertices.unmap();
	return dmx;
}

void ofxPixelMapper::setUniverse(int index, int universe) {
	int *vData = vertices.map() + index * 8;
	vData[1] = universe;
	vData[3] = universe;
	vData[5] = universe + 1;
	vData[7] = universe + 1;
	vertices.unmap();
}

void ofxPixelMapper::setDmx(int index, int dmx) {
	int *vData = vertices.map() + index * 8;
	dmx /= 3;
	vData[0] = dmx;
	vData[2] = dmx + 1;
	vData[4] = dmx + 1;
	vData[6] = dmx;
	vertices.unmap();
}

void ofxPixelMapper::saveFile(string filePath) {
	
	ofxXmlSettings xml;
	
	xml.addTag("map");
	xml.setAttribute("map", "width", width, 0);
	xml.setAttribute("map", "height", height, 0);
	xml.pushTag("map");
			
	float *tData = texCoords.map();	
	for (int i=0; i<numAreas; i++) {
		xml.addTag("area");
		xml.pushTag("area", i);
		for (int j=0; j<4; j++) {
			xml.addTag("point");
			xml.setAttribute("point", "x", tData[0], j);
			xml.setAttribute("point", "y", tData[1], j);
			tData += 2;
		}
		xml.popTag(); // area
	}
	texCoords.unmap();

	int *vData = vertices.map();
	for (int i=0; i<numAreas; i++) {
		xml.setAttribute("area", "universe", vData[1], i);
		xml.setAttribute("area", "dmx", vData[0] * 3, i);
		vData += 8;
	}
	vertices.unmap();
	
	xml.popTag(); // map

	xml.saveFile(filePath);
}

void ofxPixelMapper::loadFile(string filePath) {
	
	ofxXmlSettings xml;
	xml.loadFile(filePath);
	
	this->width = xml.getAttribute("map", "width", 640, 0);
	this->height = xml.getAttribute("map", "height", 480, 0);
	
	xml.pushTag("map");
	
	int universes = 1;
	this->numAreas = xml.getNumTags("area");
	for (int i=0; i<numAreas; i++) {
		int u = xml.getAttribute("area", "universe", 0, i);
		if (u > universes - 1)
			universes = u + 1;
	}
	allocate(numAreas, universes);
	
	float *tData = texCoords.map();	
	for (int i=0; i<numAreas; i++) {
		xml.pushTag("area", i);
		for (int j=0; j<4; j++) {
			tData[0] = xml.getAttribute("point", "x", 0., j);
			tData[1] = xml.getAttribute("point", "y", 0., j);
			tData += 2;
		}
		xml.popTag();
	}
	texCoords.unmap();

	int *vData = vertices.map();
	for (int i=0; i<numAreas; i++) {
		int u = xml.getAttribute("area", "universe", 0, i);
		int d = xml.getAttribute("area", "dmx", 0, i) / 3;
		vData[0] = d;
		vData[1] = u;
		vData[2] = d + 1;
		vData[3] = u;
		vData[4] = d + 1;
		vData[5] = u + 1;
		vData[6] = d;
		vData[7] = u + 1;
		vData += 8;
	}
	vertices.unmap();
	
	xml.popTag();
}

bool ofxPixelMapper::pointInArea(ofPoint p, mapArea* area) {
	mapPoint* points = area->points;
	bool b = false;
	float minx = HUGE_VALF;
	float miny = HUGE_VALF;
	float maxx = 0;
	float maxy = 0;
	
	for (int i=0; i<4; i++) {
		if (points[i].x < minx)
			minx = points[i].x;
		if (points[i].y < miny)
			miny = points[i].y;
		if (points[i].x > maxx)
			maxx = points[i].x;
		if (points[i].y > maxy)
			maxy = points[i].y;
	}
	
	b = p.x >= minx && p.x <= maxx && p.y >= miny && p.y <= maxy;
	
	if (b) {
		bool c = false;
		int j = 3;
		for (int i=0; i<4; j=i++) {
			if ( ((points[i].y>p.y) != (points[j].y>p.y)) &&
				(p.x < (points[j].x-points[i].x) * (p.y-points[i].y) / (points[j].y-points[i].y) + points[i].x) )
				c = !c;
		}
		return c;
	}
	return false;
}
