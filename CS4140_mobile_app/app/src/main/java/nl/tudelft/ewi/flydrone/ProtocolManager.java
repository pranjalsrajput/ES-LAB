package nl.tudelft.ewi.flydrone;

/**
 * Author:Pranjal Singh Rajput
 * Description: Implementation of Communication protocol on mobile side
 */

import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class ProtocolManager {


    public static final int COMM_LENGTH_STATE_REQUEST = 0;
    public static final int COMM_LENGTH_STATE_RESPONSE = 2;
    public static final int COMM_LENGTH_SENSOR_REQUEST = 0;
    public static final int COMM_LENGTH_SENSOR_RESPONSE = 4;
    public static final int COMM_LENGTH_CONTROL = 7;
    public static final int COMM_LENGTH_MODE = 1;
    public static final int COMM_LENGTH_HEIGHT = 1;
    public static final int COMM_LENGTH_CHANNELSWITCH = 1;
    public static final int COMM_LENGTH_LOGGING_CONTROL = 1;
    public static final int COMM_LENGTH_LOGGING_SIZE_REQUEST = 0;

    public static final int SIDE_PC = 1;
    public static final int SIDE_MC = 2;

    public static final int SIDE_THIS = SIDE_PC;
    public static final int SIDE_OTHER = SIDE_MC;

    byte[] buffer = new byte[128];
    int bufferLen = 0;

    int indexOf(byte what) {
        for (int i = 0; i < bufferLen; i++) {
            if (buffer[i] == what) {
                return i;
            }
        }
        return -1;
    }

    void messageReceived(byte[] message, int length) {

    }

    void removeFromBufferStart(int number) {
        for (int i = 0; i < bufferLen - number - 1; i++) {
            buffer[i] = buffer[i + number];
        }
        bufferLen -= number;
    }

    void processIncoming(byte[] inBuffer, int inLen) {
        for (int i = 0; i < inLen; i++) {
            if (bufferLen < 128) {
                buffer[bufferLen] = inBuffer[i];
                bufferLen++;
            }
        }
        int indStart = indexOf(c2b('['));
        int indEnd;
        int side = SIDE_OTHER;
        if (bufferLen > indStart + 1) {
            indEnd = indStart + 1 + getMessageLength(inBuffer[indStart + 1], side) + 1;
        } else {
            indEnd = -1;
        }
        while (indStart > -1 && indEnd > -1 && indEnd <= bufferLen) {
            if (indStart <= indEnd) {
                int messageLength = getMessageLength(buffer[indStart + 1], side);
                if (bufferLen > indStart + messageLength + 2) {
                    boolean checksumOk = false;
                    int checksum = 0;
                    for (int i = 0; i < 2 + messageLength; i++) {
                        checksum += buffer[indStart + i];
                    }
                    byte cchecksum = (byte) (checksum % 128);
                    if (cchecksum == buffer[indStart + messageLength + 2]) {
                        checksumOk = true;
                    } else {
                        checksumOk = false;
                    }
                    if (!checksumOk) {
                        removeFromBufferStart(indStart + 1);
                    } else {
                        byte[] newBuffer = Arrays.copyOfRange(buffer, indStart + 1, buffer.length);
                        messageReceived(newBuffer, messageLength + 1);
                        removeFromBufferStart(indStart + 2 + messageLength + 1);
                    }
                } else {
                    break;
                }
            } else {
                removeFromBufferStart(indEnd + 1);
            }
            indStart = indexOf(c2b('['));
            if (bufferLen > indStart + 1) {
                indEnd = indStart + 1 + getMessageLength(inBuffer[indStart + 1], side) + 1;
            } else {
                indEnd = -1;
            }
        }
    }

    public static byte c2b(char c) {
        return StandardCharsets.ISO_8859_1.encode(new String(new char[]{c})).get(0);
    }

    public static byte i2b(int c) {
        return c > 127 ? 127 : c < -127 ? -127 : (byte) c;
    }

    static public int getMessageLength(byte action, int side) {
        switch (action) {
            case 's':
                return side == SIDE_PC ? COMM_LENGTH_STATE_REQUEST : COMM_LENGTH_STATE_RESPONSE;
            case 'a':
                return side == SIDE_PC ? COMM_LENGTH_SENSOR_REQUEST : COMM_LENGTH_SENSOR_RESPONSE;
            case 'm':
                return COMM_LENGTH_CONTROL;
            case 'c':
                return COMM_LENGTH_MODE;
            case 'h':
                return COMM_LENGTH_HEIGHT;
            case 'w':
                return COMM_LENGTH_CHANNELSWITCH;
            case 'l':
                return COMM_LENGTH_LOGGING_CONTROL;
            case 'b':
                return side == SIDE_PC ? COMM_LENGTH_LOGGING_SIZE_REQUEST : COMM_LENGTH_STATE_RESPONSE;
            default:
                return 0;
        }
    }

    public static byte[] constructMessage(byte action, byte[] params) {
        byte[] output = new byte[200];
        output[0] = '[';
        output[1] = action;
        int paramsLen = getMessageLength(action, SIDE_THIS);
        for (int i = 0; i < paramsLen; i++) {
            output[2 + i] = params[i];
        }
        int checksum = 0;
        for (int i = 0; i < 2 + paramsLen; i++) {
            checksum += output[i];
        }
        output[2 + paramsLen] = (byte) (checksum % 128);
        output[2 + paramsLen + 1] = c2b('\n');
        int outLen = 2 + paramsLen + 1 + 1;
        byte[] ret = new byte[outLen];
        System.arraycopy(output, 0, ret, 0, outLen);
        return ret;
    }


}
