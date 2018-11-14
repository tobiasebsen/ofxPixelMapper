//
//  ofxPixelMapper.h
//
//  Created by Tobias Ebsen on 19/06/17.
//
//

#pragma once

#include "ofMain.h"

class ofxPixelMapper {
public:
    
    typedef enum {
        RGB,
        RGBW
    } PixelMode;
    
    ofxPixelMapper();

    // Set up internal frame buffer
    void setup(PixelMode mode = RGB, int numUniverses = 1, int numChannels = 512);

    // Add mapping
    void addQuad(int pixel, int universe, ofVec2f v1, ofVec2f v2, ofVec2f v3, ofVec2f v4, ofFloatColor calib = ofFloatColor::white);
    void addQuad(int pixel, int universe, float x1, float y1, float x2, float y2);
    void addQuad(int pixel, int universe, ofRectangle rect);
    void addGrid(int pixel, int universe, float x1, float y1, float x2, float y2, int hcount, int vcount);
    void addGrid(int pixel, int universe, ofRectangle rect, int hcount, int vcount);
    void updateMapping();
    void normalizeMapCoords(int width, int height);
    
    int getNumUniverses();
    int getNumPixels();
    int getNumChannelsPerUniverse();
    int getNumPixelsPerUniverse();
    
    void setBrightness(float bright);
    void setUseCalibration(bool useCalib);
    
    // Transfer texture data to pixels
    void update(ofTexture & tex);
    
    // Draw the internal frame buffer
    void draw(float x, float y, float scale = 1.f);
    
    // Draw mapping
    void drawMapping();
    void drawMapping(ofTexture & tex);
    
    // Read pixel data
    void read(int universe, unsigned char * data, int byteLength);
    void read(ofPixels & pixels);

protected:
    
    ofFbo fbo;
    ofVbo vbo;
    ofMesh mesh;
    vector<ofVec2f> normalized;

    PixelMode mode;
    int numUniverses;
    int numChannelsPerUniverse;
    int numChannelsPerPixel;
    int numPixelsPerUniverse;
    int numPixelsTotal;
    
    bool useCalib;
    bool useNormalized;
    float brightness;
};