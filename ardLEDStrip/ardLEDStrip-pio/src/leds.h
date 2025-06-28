void TurnOffAll()
{
    infoMessln("Turning all off");
    strip.clear(); // Set all pixel colors to 'off'
    strip.show();
}

/*void colorWipe(uint32_t color, int WAIT)
{
    for (int i = 0; i < strip.numPixels(); i++)
    {                                  // For each pixel in strip...
        strip.setPixelColor(i, color); //  Set pixel's color (in RAM)
        strip.show();                  //  Update strip to match
        delay(WAIT);                   //  Pause for a moment
    }
}*/

void red()
{
    //colorWipe(strip.Color(255, 0, 0), 50); // Red
}

void blue()
{
    //colorWipe(strip.Color(0, 255, 0), 50);
}

void green()
{
    //colorWipe(strip.Color(0, 0, 255), 50);
}

void TrueWhite()
{
    infoMessln("White");
    // colorWipe(strip.Color(0, 0, 0, 255), 50);
    for (int j = 0; j < LED_COUNT; j++)
    { // Ramp up from 0 to 255
        //infoMessln(j);
        // strip.fill(strip.Color(255, 255, 255), 50);
        strip.setPixelColor(j, (127, 127, 127));
        strip.show();
    }
}