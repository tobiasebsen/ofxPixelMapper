/*
 *  ofxPixelMapper.h
 *
 *  Created by Tobias Ebsen on 6/7/13.
 *
 */

#pragma once

#include "ofxVertexBuffer.h"
#include "ofxTexCoordBuffer.h"
#include "ofxColorBuffer.h"
#include "ofxXmlSettings.h"

struct mapPoint {
	float x;
	float y;
}__attribute__((packed));

struct mapArea {
	mapPoint points[4];
}__attribute__((packed));

class ofxPixelMapper {
public:
	
	void setup(int width, int height);
	void allocate(int numAreas, int numUniverses = 1);
	
	void setArea(int index, mapArea & area);
	void setArea(int index, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
	void setArea(int index, ofPoint p1, ofPoint p2, ofPoint p3, ofPoint p4);
	
	mapArea getArea(int index);
	void getArea(int index, ofPoint& p1, ofPoint& p2, ofPoint& p3, ofPoint& p4);
	
	ofPoint getAreaCenter(int index);
	int findArea(float x, float y);
	int findByDmx(int index, float threshold, int dmx);
	int findByUniverse(int index, float threshold, int universe);
	
	void updatePixels(ofTexture & tex);
	void copyPixels(int universe, unsigned char* data, int length);
	
	void drawColors(ofTexture & tex);
	
	void drawAreas(int mode); // GL_LINES, GL_POINTS, GL_QUADS
	void drawArea(int index, int mode); // GL_LINES, GL_POINTS, GL_QUADS
	
	void setSize(int width, int height);
	
	int getWidth();
	int getHeight();
	int getNumAreas();
	int getNumUniverses();
	
	void setUniverse(int index, int universe);
	void setDmx(int index, int dmx);
	
	int getUniverse(int index);
	int getDmx(int index);
	
	void saveFile(string filePath);
	void loadFile(string filePath);
	
private:
	
	bool pointInArea(ofPoint p, mapArea* area);
	
	int width;
	int height;
	int numAreas;
	int numUniverses;
	
	ofxVertexBufferInt vertices;
	ofxTexCoordBufferFloat texCoords;
	ofxColorBufferUbyte pixels;
	ofFbo fboPixels;
	
	ofxVertexBufferInt colorPos;
	ofxColorBufferUbyte colors;
	ofFbo fboColors;
};