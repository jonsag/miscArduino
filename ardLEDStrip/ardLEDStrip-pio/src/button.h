#include <OneButton.h>

OneButton btn = OneButton(
    ButtonPin, // Input pin for the button
    false,     // Button is active high
    false      // Disable internal pull-up resistor
);

void action()
{
    infoMess("Status: ");
    infoMessln(status);

    switch (status)
    {
    case 0:
    {
        TurnOffAll();
    }
    break;
    case 1:
    {
        white();
    }
    break;
    case 2:
    {
        red();
    }
    break;
    case 3:
    {
        blue();
    }
    break;
    case 4:
    {
        green();
    }
    break;
    case 5:
    {
        //RunDemo();
    }
    break;
    }

infoMessln();
}

// Handler function for a single click:
void HandleClick()
{
    infoMessln("Clicked!");

    if (status >= 1)
    {
        status = 0;
    }
    else
    {
        status = 1;
    }

    action();
}

void HandleLongPress()
{
    infoMessln("Long press!");

    status = 5;

    infoMess("Status: ");
    infoMessln(status);
     action();
}

void HandleDoubleClick()
{
    infoMessln("Double click!");

    if (status == 0)
    {
        status = 1;
    }

    status++;

    if (status > 4)
    {
        status = 2;
    }

    infoMess("Status: ");
    infoMessln(status);
    infoMessln();

    action();
}
