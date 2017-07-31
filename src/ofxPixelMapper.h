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

    void setup(PixelMode mode = RGB, int numUniverses = 1, int numChannels = 512);

    void addQuad(int pixel, int universe, float x1, float y1, float x2, float y2);
    void addGrid(int pixel, int universe, float x1, float y1, float x2, float y2, int hcount, int vcount);
    void updateMapping();
    
    int getNumUniverses();
    
    void update(ofTexture & tex);
    void draw(float x, float y, float w, float h);
    void read(int universe, unsigned char * data);

protected:
    
    ofFbo fbo;
    ofVbo vbo;
    ofMesh mesh;

    PixelMode mode;
    int numUniverses;
    int numChannelsPerUniverse;
    int numChannelsPerPixel;
    int numPixelsPerUniverse;
};