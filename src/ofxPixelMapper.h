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

    // Set up internal frame buffer
    void setup(PixelMode mode = RGB, int numUniverses = 1, int numChannels = 512);

    // Add mapping
    void addQuad(int pixel, int universe, float x1, float y1, float x2, float y2);
    void addGrid(int pixel, int universe, float x1, float y1, float x2, float y2, int hcount, int vcount);
    void updateMapping();
    
    int getNumUniverses();
    int getNumPixels();
    
    // Transfer texture data to pixels
    void update(ofTexture & tex);
    
    // Draw the internal frame buffer
    void draw(float x, float y);
    void draw(float x, float y, float w, float h);
    
    // Read pixel data
    void read(int universe, unsigned char * data, int byteLength);

protected:
    
    ofFbo fbo;
    ofVbo vbo;
    ofMesh mesh;

    PixelMode mode;
    int numUniverses;
    int numChannelsPerUniverse;
    int numChannelsPerPixel;
    int numPixelsPerUniverse;
    int numPixelsTotal;
};