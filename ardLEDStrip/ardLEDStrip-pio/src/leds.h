void TurnOffAll()
{
    infoMessln("Turning all off");
    strip.clear(); // Set all pixel colors to 'off'
    strip.show();
}

void colorWipe(uint32_t color, int WAIT)
{
    for (int i = 0; i < strip.numPixels(); i++)
    {                                  // For each pixel in strip...
        strip.setPixelColor(i, color); //  Set pixel's color (in RAM)
        strip.show();                  //  Update strip to match
        delay(WAIT);                   //  Pause for a moment
    }
}

void red()
{
    infoMessln("Red");

    colorWipe(strip.Color(255, 0, 0), 50); // Red

    /*color = (127, 0, 0);
    for (int j = 0; j < LED_COUNT; j++)
    {
        strip.setPixelColor(j, color);
        strip.show();
    }*/
}

void blue()
{
    infoMessln("Blue");

    colorWipe(strip.Color(0, 0, 255), 50);

    /*color = (0, 255, 0);
    for (int j = 0; j < LED_COUNT; j++)
    {
        strip.setPixelColor(j, color);
        strip.show();
    }*/
}

void green()
{
    infoMessln("Green");

    colorWipe(strip.Color(0, 255, 0), 50);

    /*color = (0, 0, 127);
    for (int j = 0; j < LED_COUNT; j++)
    {
        strip.setPixelColor(j, color);
        strip.show();
    }*/
}

void TrueWhite()
{
    infoMessln("White");

    colorWipe(strip.Color(255, 255, 255), 50);

    /*color = (127, 127, 127);
    for (int j = 0; j < LED_COUNT; j++)
    {
        strip.setPixelColor(j, color);
        strip.show();
    }*/
}