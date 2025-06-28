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
        TrueWhite();
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

    status = 0;

    infoMess("Status: ");
    infoMessln(status);
    infoMessln();
}

void HandleDoubleClick()
{
    infoMessln("Double click!");

    status++;

    infoMess("Status: ");
    infoMessln(status);
    infoMessln();
}
