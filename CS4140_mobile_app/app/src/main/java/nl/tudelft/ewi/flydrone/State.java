package nl.tudelft.ewi.flydrone;
/**
 * Author:Pranjal Singh Rajput
 * Description: To set the control param values in different modes/states.
 */
public class State {
    char mode = '-';
    //Full Control Mode
    int controllerPYawKeyboard = 1;
    int controllerP1RollPitchKeyboard = 5;
    int controllerP2RollPitchKeyboard = 20;
    //Raw mode
    /*int controllerPYawKeyboard = 1;
    int controllerP1RollPitchKeyboard = 7;
    int controllerP2RollPitchKeyboard = 40;*/
    int liftKeyboard = 0;
    int rollKeyboard = 0;
    int pitchKeyboard = 0;
    int yawKeyboard = 0;
};