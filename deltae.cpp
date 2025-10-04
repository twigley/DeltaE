#define _USE_MATH_DEFINES

#include <iostream>
#include <cmath>
#include <string>

struct rgb_colour
{
    int r;
    int g;
    int b;
};

struct xyz_colour
{
    double x;
    double y;
    double z;
};

struct lab_colour
{
    double L;
    double a;
    double b;
};

xyz_colour rgb_to_xyz(rgb_colour colour)
{
    // D65/2° standard illuminant
    xyz_colour xyz;
    
    double r = colour.r/255;
    double g = colour.g/255;
    double b = colour.b/255;

    r = (r > 0.04045) ? pow( ( r + 0.055 ) / 1.055, 2.4) : r / 12.92;
    g = (g > 0.04045) ? pow( ( g + 0.055 ) / 1.055, 2.4) : g / 12.92;
    b = (b > 0.04045) ? pow( ( b + 0.055 ) / 1.055, 2.4) : b / 12.92;

    r = r * 100;
    g = g * 100;
    b = b * 100;

    xyz.x = r * 0.4124 + g * 0.3576 + b * 0.1805;
    xyz.y = r * 0.2126 + g * 0.7152 + b * 0.0722;
    xyz.z = r * 0.0193 + g * 0.1192 + b * 0.9505;

    return xyz;
}

lab_colour xyz_to_lab(xyz_colour xyz){
    lab_colour lab;

    // https://en.wikipedia.org/wiki/Standard_illuminant 
    const double ReferenceX = 92.834;
    const double ReferenceY = 100.000;
    const double ReferenceZ = 103.665;
    
    // apply reference corrections for sRGB https://www.color.org/srgb.pdf which uses d65
    double var_X = xyz.x / ReferenceX;
    double var_Y = xyz.y / ReferenceY;
    double var_Z = xyz.z / ReferenceZ;

    var_X = ( var_X > 0.008856 ) ? pow(var_X, 1.0/3.0) : ( 7.787 * var_X ) + ( 16 / 116 );
    var_Y = ( var_Y > 0.008856 ) ? pow(var_Y, 1.0/3.0) : ( 7.787 * var_Y ) + ( 16 / 116 );
    var_Z = ( var_Z > 0.008856 ) ? pow(var_Z, 1.0/3.0) : ( 7.787 * var_Z ) + ( 16 / 116 );

    lab.L = ( 116 * var_Y ) - 16;
    lab.a = 500 * ( var_X - var_Y );
    lab.b = 200 * ( var_Y - var_Z );
 
    return lab;
}

lab_colour rgb_to_lab(rgb_colour rgb){
    return xyz_to_lab(rgb_to_xyz(rgb));
}

double deltaE(lab_colour a, lab_colour b){
    // CIE 1976
    // https://en.wikipedia.org/wiki/Color_difference#CIE76
    return sqrt(pow(b.L-a.L,2)+pow(b.a-a.a,2)+pow(b.b-a.b,2));
}

double hue_from_lab(lab_colour lab){
    double hue;
    if (lab.a == 0 && lab.b == 0){
        hue = 0; 
    }
    else if (lab.a >= 0){
        hue = atan2(lab.b,lab.a);
    }
    else {
        hue = atan2(lab.b, lab.a) + 2 * M_PI;
    }
    return hue;
}

double deltaE2000(lab_colour a, lab_colour b){
    double dL = b.L - a.L;
    double L_ = (a.L + b.L) / 2;
    double C1 = sqrt(pow(a.a,2) + pow(b.a,2));
    double C2 = sqrt(pow(a.b,2) + pow(b.b,2));
    double C_ = (C1 + C2) / 2;
    double dC = C2 - C1;
    double h1 = hue_from_lab(a);
    double h2 = hue_from_lab(b);
    double kL = 1, kC = 1, kH = 1;
    double SC = 1 + 0.045 * C_;
    double SL = 1 + 0.015*pow(L_-50,2)/sqrt(20+pow(L_-50,2));

    double h_;
    try{
        if (h1-h2 <= 180){
            h_ = (h1+h2)/2;
        }
        else if (h1-h2 > 180 && h1-h2 < 360){
            h_ = (h1+h2+360)/2;
        }
        else if (h1-h2 > 180 && h1-h2 >= 360){
            h_ = (h1+h2-360)/2;
        }
        else {
            throw(h_);
        }
    }
    catch (...){
        std::cout << "something went wrong sorting out h_";
    }
    double dh = 0; //todo

    double T = 1 - 0.17 * cos(1);
    double SH = 1 + 0.015*C_*T;

    double dE = sqrt(pow(dL/kL*SL,2)+pow(dC/kC*SC,2)+pow(dH/kH*sH,2)+RT*(dC/kC*SC)*(dH/kH*SH));
}

int main()
{
    double deltaresult;

    rgb_colour red = {255,0,0};
    rgb_colour blue = {0,0,255};

    deltaresult = deltaE(rgb_to_lab(red), rgb_to_lab(blue));

    std::cout << deltaresult << std::endl;
}