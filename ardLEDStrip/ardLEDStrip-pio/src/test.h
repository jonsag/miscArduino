void test()
{
    int step = 10;
    
    infoMessln("Running test...");

    for (int red = 0; red < 256; red= red + step)
    {
        for (int blue = 0; blue < 256; blue = blue + step)
        {
            for (int green = 0; green < 256; green = green + step)
            {
                infoMess(red);
                infoMess("\t");
                infoMess(blue);
                infoMess("\t");
                infoMessln(green);

                for (int j = 0; j < LED_COUNT; j++)
                {

                    strip.setPixelColor(j, (red, green, blue));
                    strip.show();
                }
            }
        }
    }
    infoMessln();
}