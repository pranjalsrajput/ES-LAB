/**
 * Author:Pranjal Singh Rajput
 * Description: Main Class of the android app containing all required methods
 * Using the flydrone app you can connect the the drone to the mobile first.
 * Then can start the communication between phone and drone by selecting the checkbox.
 * All Modes can be selected from the dropdown.
 * Seek bar is used to control the lift of drone
 * Android phone's motion gestures are used to control the Rolling, Pitching and Yawing of the drone.
 * Calibrate yaw button is there to reset roll, pitch and yaw values.
 * Value of roll, pitch, and yaw is displayed on the mobile, along with the direction and degree of rotation of the mobile.
 */
package nl.tudelft.ewi.flydrone;

import android.Manifest;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.content.LocalBroadcastManager;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Display;
import android.view.Surface;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.RadioGroup;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import java.text.DateFormat;
import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;

import static nl.tudelft.ewi.flydrone.ProtocolManager.c2b;
import static nl.tudelft.ewi.flydrone.ProtocolManager.constructMessage;
import static nl.tudelft.ewi.flydrone.ProtocolManager.i2b;


/**
 * Author:Pranjal Singh Rajput
 * Description: Main class for the app.
 */
public class MainActivity extends AppCompatActivity implements RadioGroup.OnCheckedChangeListener, SensorEventListener {
    private static final int REQUEST_SELECT_DEVICE = 1;
    private static final int REQUEST_ENABLE_BT = 2;
    public static final String TAG = "nRFUART";
    private static final int UART_PROFILE_CONNECTED = 20;
    private static final int UART_PROFILE_DISCONNECTED = 21;


    private int mState = UART_PROFILE_DISCONNECTED;
    private UartService mService = null;
    private BluetoothDevice mDevice = null;
    private BluetoothAdapter mBtAdapter = null;
    private Button btnConnectDisconnect;
    private Button btnPanic,buttonCalibrateYaw;

    private Spinner spinnerMode;

    /**
     * The sensor manager object.
     */
    private SensorManager sensorManager;
    /**
     * The accelerometer.
     */
    private Sensor accelerometer;
    private Sensor mSensorMagnetometer;

    // Current data from accelerometer & magnetometer.  The arrays hold values
    // for X, Y, and Z.
    private float[] mAccelerometerData = new float[3];
    private float[] mMagnetometerData = new float[3];

    // System display. Need this for determining rotation.
    private Display mDisplay;

    private float aX = 0;
    /**
     * Accelerometer y value
     */
    private float aY = 0;
    /**
     * Accelerometer z value
     */
    private float aZ = 0;
    private float azimuth = 0;
    private float azimuthDeg = 0;
    private float defaultAzimuthDeg = 0;
    private int calculatedYaw=0;
    private int calculatedRoll=0;
    private int calculatedPitch=0;
    private float defaultRoll = 0;
    private float defaultPitch= 0;
    private float pitch = 0;
    private float roll = 0;

    /**
     * Text fields to show the sensor values.
     */
    private TextView currentX, currentY, currentZ, titleAcc,mTextSensorAzimuth,value_direction;

    private SeekBar seekLift;

    public void changeState(char charState) {
        String s = "";
        s += charState;
        Log.d("state change", s);
        mService.sendMessage(constructMessage(c2b('c'), new byte[]{c2b(charState)}));
    }

    /**
     * Author:Pranjal Singh Rajput
     * Description: Method to register all sensors, initializa buletooth adaptor and activate the views.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        this.requestPermissions(new String[]{Manifest.permission.ACCESS_COARSE_LOCATION}, 111);
        this.requestPermissions(new String[]{Manifest.permission.BLUETOOTH}, 111);
        this.requestPermissions(new String[]{Manifest.permission.BLUETOOTH_ADMIN}, 111);

        // Create the text views.
        currentX = (TextView) findViewById(R.id.currentX);
        currentY = (TextView) findViewById(R.id.currentY);
        currentZ = (TextView) findViewById(R.id.currentZ);
        titleAcc = (TextView) findViewById(R.id.titleAcc);
        value_direction = (TextView) findViewById(R.id.value_direction);
        mTextSensorAzimuth = (TextView) findViewById(R.id.value_azimuth);


        seekLift = findViewById(R.id.seekLift);
        spinnerMode = findViewById(R.id.spinnerMode);
        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,
                R.array.modes, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinnerMode.setAdapter(adapter);
        spinnerMode.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                if (mDevice == null) {
                    return;
                }
                String[] sp = getResources().getStringArray(R.array.modes)[position].split(":");
                String a = sp[sp.length - 1].trim();
                char charState = a.toCharArray()[0];
                changeState(charState);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });


        // Set the sensor manager
        sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);


        sensorManager = (SensorManager) getSystemService(
                Context.SENSOR_SERVICE);
        accelerometer = sensorManager.getDefaultSensor(
                Sensor.TYPE_ACCELEROMETER);
        mSensorMagnetometer = sensorManager.getDefaultSensor(
                Sensor.TYPE_MAGNETIC_FIELD);

        if (accelerometer != null) {
            sensorManager.registerListener(this, accelerometer,
                    SensorManager.SENSOR_DELAY_GAME);
        }
        if (mSensorMagnetometer != null) {
            sensorManager.registerListener(this, mSensorMagnetometer,
                    SensorManager.SENSOR_DELAY_GAME);
        }
        // Get the display from the window manager (for rotation).
        WindowManager wm = (WindowManager) getSystemService(WINDOW_SERVICE);
        mDisplay = wm.getDefaultDisplay();

        mBtAdapter = BluetoothAdapter.getDefaultAdapter();
        if (mBtAdapter == null) {
            Toast.makeText(this, "Bluetooth is not available", Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        btnConnectDisconnect = (Button) findViewById(R.id.buttonConnect);
        btnPanic = (Button) findViewById(R.id.buttonPanic);
        buttonCalibrateYaw=(Button)findViewById(R.id.buttonCalibrateYaw);
        //Calibrate the yaw values
        buttonCalibrateYaw.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                float orientationValues[] = new float[3];
                orientationValues=getOrientation();
                // Pull out the individual values from the array.
                defaultAzimuthDeg=(float)Math.toDegrees(orientationValues[0]);
                defaultPitch=orientationValues[1];
                defaultRoll=orientationValues[2];
                System.out.println("default azimuth= "+defaultAzimuthDeg);
            }
        });
        btnPanic.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                changeState('p');
            }
        });

        service_init();


        // Handle Disconnect & Connect button
        btnConnectDisconnect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!mBtAdapter.isEnabled()) {
                    Log.i(TAG, "onClick - BT not enabled yet");
                    Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                    startActivityForResult(enableIntent, REQUEST_ENABLE_BT);
                } else {
                    if (btnConnectDisconnect.getText().equals("Connect")) {

                        //Connect button pressed, open DeviceListActivity class, with popup windows that scan for devices

                        Intent newIntent = new Intent(MainActivity.this, DeviceListActivity.class);
                        startActivityForResult(newIntent, REQUEST_SELECT_DEVICE);
                    } else {
                        //Disconnect button pressed
                        if (mDevice != null) {
                            mService.disconnect();

                        }
                    }
                }
            }
        });
    }

    Timer timer;

    /**
     * Author:Beni Kovacs
     * Acknowledgement: this function is adapted from the one found in the code samples of Nordic
     * see: https://github.com/NordicPlayground/Android-nRF-UART
     * Description: UART service connected/disconnected
     */
    private ServiceConnection mServiceConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder rawBinder) {
            mService = ((UartService.LocalBinder) rawBinder).getService();
            Log.d(TAG, "onServiceConnected mService= " + mService);
            if (!mService.initialize()) {
                Log.e(TAG, "Unable to initialize Bluetooth");
                finish();
            }

        }

        public void onServiceDisconnected(ComponentName classname) {
            ////     mService.disconnect(mDevice);
            mService = null;
        }
    };

   /* private Handler mHandler = new Handler() {
        @Override

        //Handler events that received from UART service
        public void handleMessage(Message msg) {

        }
    };*/

    /**
     * Author:Beni Kovacs
     * Acknowledgement: this class is adapted from the one found in the code samples of Nordic
     * see: https://github.com/NordicPlayground/Android-nRF-UART
     * Description: UART service connected/disconnected
     */
    private final BroadcastReceiver UARTStatusChangeReceiver = new BroadcastReceiver() {

        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            final Intent mIntent = intent;
            //*********************//
            if (action.equals(UartService.ACTION_GATT_CONNECTED)) {
                runOnUiThread(new Runnable() {
                    public void run() {


                        // Set initial UI state

                        timer = new Timer();
                        timer.schedule(new TimerTask() {
                            @Override
                            public void run() {
                                timerExpired();
                            }
                        }, 50, 200);

                        String currentDateTimeString = DateFormat.getTimeInstance().format(new Date());
                        Log.d(TAG, "UART_CONNECT_MSG");
                        btnConnectDisconnect.setText("Disconnect");

                        ((TextView) findViewById(R.id.deviceName)).setText(mDevice.getName() + " - ready");

                        mState = UART_PROFILE_CONNECTED;
                    }
                });
            }

            //*********************//
            if (action.equals(UartService.ACTION_GATT_DISCONNECTED)) {
                runOnUiThread(new Runnable() {
                    public void run() {

                        if (timer != null) {
                            timer.cancel();
                        }


                        String currentDateTimeString = DateFormat.getTimeInstance().format(new Date());
                        Log.d(TAG, "UART_DISCONNECT_MSG");
                        btnConnectDisconnect.setText("Connect");

                        ((TextView) findViewById(R.id.deviceName)).setText("Not Connected");
                        mState = UART_PROFILE_DISCONNECTED;
                        mService.close();
                        //setUiState();

                    }
                });
            }


            //*********************//
            if (action.equals(UartService.ACTION_GATT_SERVICES_DISCOVERED)) {
                mService.enableTXNotification();
            }
            //*********************//
            if (action.equals(UartService.ACTION_DATA_AVAILABLE)) {

                final byte[] txValue = intent.getByteArrayExtra(UartService.EXTRA_DATA);
                runOnUiThread(new Runnable() {
                    public void run() {
                        try {
                            String text = new String(txValue, "UTF-8");
                            String currentDateTimeString = DateFormat.getTimeInstance().format(new Date());


                        } catch (Exception e) {
                            Log.e(TAG, e.toString());
                        }
                    }
                });
            }
            //*********************//
            if (action.equals(UartService.DEVICE_DOES_NOT_SUPPORT_UART)) {
                showMessage("Device doesn't support UART. Disconnecting");
                mService.disconnect();
            }
        }
    };

    /**
     * Author:Beni Kovacs
     * Acknowledgement: this function is adapted from the one found in the code samples of Nordic
     * see: https://github.com/NordicPlayground/Android-nRF-UART
     * Description: UART service initialize
     */
    private void service_init() {
        Intent bindIntent = new Intent(this, UartService.class);
        bindService(bindIntent, mServiceConnection, Context.BIND_AUTO_CREATE);

        LocalBroadcastManager.getInstance(this).registerReceiver(UARTStatusChangeReceiver, makeGattUpdateIntentFilter());
    }

    /**
     * Author:Beni Kovacs
     * Acknowledgement: this function is adapted from the one found in the code samples of Nordic
     * see: https://github.com/NordicPlayground/Android-nRF-UART
     * Description:update intent filter
     */
    private static IntentFilter makeGattUpdateIntentFilter() {
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(UartService.ACTION_GATT_CONNECTED);
        intentFilter.addAction(UartService.ACTION_GATT_DISCONNECTED);
        intentFilter.addAction(UartService.ACTION_GATT_SERVICES_DISCOVERED);
        intentFilter.addAction(UartService.ACTION_DATA_AVAILABLE);
        intentFilter.addAction(UartService.DEVICE_DOES_NOT_SUPPORT_UART);
        return intentFilter;
    }

    /**
     * Author:Pranjal Singh Rajput
     */
    @Override
    public void onStart() {
        super.onStart();
    }

    /**
     * Author:Pranjal Singh Rajput
     */
    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy()");

        try {
            LocalBroadcastManager.getInstance(this).unregisterReceiver(UARTStatusChangeReceiver);
        } catch (Exception ignore) {
            Log.e(TAG, ignore.toString());
        }
        unbindService(mServiceConnection);
        mService.stopSelf();
        mService = null;

    }

    /**
     * Author:Pranjal Singh Rajput
     * Description: To unregister sensors on device stop
     */
    @Override
    protected void onStop() {
        Log.d(TAG, "onStop");
        super.onStop();
        sensorManager.unregisterListener(this);
    }

    /**
     * Author:Pranjal Singh Rajput
     */
    @Override
    protected void onPause() {
        Log.d(TAG, "onPause");
        super.onPause();
        sensorManager.unregisterListener(this);
    }

    /**
     * Author:Pranjal Singh Rajput
     */
    @Override
    protected void onRestart() {
        super.onRestart();
        Log.d(TAG, "onRestart");
    }

    /**
     * Author:Pranjal Singh Rajput
     * Description: To register sensors on sensor resume
     */
    @Override
    public void onResume() {
        super.onResume();
        sensorManager.registerListener(this, accelerometer,
                SensorManager.SENSOR_DELAY_GAME);
        sensorManager.registerListener(this, mSensorMagnetometer,
                SensorManager.SENSOR_DELAY_GAME);
        Log.d(TAG, "onResume");
        if (!mBtAdapter.isEnabled()) {
            Log.i(TAG, "onResume - BT not enabled yet");
            Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableIntent, REQUEST_ENABLE_BT);
        }

    }

    /**
     * Author:Pranjal Singh Rajput
     */
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    /**
     * Author:Pranjal Singh Rajput
     * Description: Request for selecting the device for bluetooth connection
     */
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode) {

            case REQUEST_SELECT_DEVICE:
                //When the DeviceListActivity return, with the selected device address
                if (resultCode == Activity.RESULT_OK && data != null) {
                    String deviceAddress = data.getStringExtra(BluetoothDevice.EXTRA_DEVICE);
                    mDevice = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(deviceAddress);

                    Log.d(TAG, "... onActivityResultdevice.address==" + mDevice + "mserviceValue" + mService);
                    ((TextView) findViewById(R.id.deviceName)).setText(mDevice.getName() + " - connecting");
                    mService.connect(deviceAddress);


                }
                break;
            case REQUEST_ENABLE_BT:
                // When the request to enable Bluetooth returns
                if (resultCode == Activity.RESULT_OK) {
                    Toast.makeText(this, "Bluetooth has turned on ", Toast.LENGTH_SHORT).show();

                } else {
                    // User did not enable Bluetooth or an error occurred
                    Log.d(TAG, "BT not enabled");
                    Toast.makeText(this, "Problem in BT Turning ON ", Toast.LENGTH_SHORT).show();
                    finish();
                }
                break;
            default:
                Log.e(TAG, "wrong request code");
                break;
        }
    }

    /**
     * Author:Pranjal Singh Rajput
     */
    @Override
    public void onCheckedChanged(RadioGroup group, int checkedId) {

    }

    /**
     * Author:Pranjal Singh Rajput
     */
    private void showMessage(String msg) {
        Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();

    }

    /**
     * Author:Beni Kovacs
     * Acknowledgement: this function is adapted from the one found in the code samples of Nordic
     * see: https://github.com/NordicPlayground/Android-nRF-UART
     */
    @Override
    public void onBackPressed() {
        if (mState == UART_PROFILE_CONNECTED) {
            Intent startMain = new Intent(Intent.ACTION_MAIN);
            startMain.addCategory(Intent.CATEGORY_HOME);
            startMain.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(startMain);
            showMessage("nRFUART's running in background.\n             Disconnect to exit");
        } else {

        }
    }

    /**
     * Author:Pranjal Singh Rajput
     */
    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // Do nothing.
    }

    /**
     * Author:Pranjal Singh Rajput
     * Description: Method to calculate the control values on change of the sensors.
     */
    @Override
    public void onSensorChanged(SensorEvent event) {
        currentX.setText("0.0");
        currentY.setText("0.0");
        currentZ.setText("0.0");
        value_direction.setText("origin");

        int sensorType = event.sensor.getType();
        switch (sensorType) {
            case Sensor.TYPE_ACCELEROMETER:
                mAccelerometerData = event.values.clone();
                break;
            case Sensor.TYPE_MAGNETIC_FIELD:
                mMagnetometerData = event.values.clone();
                break;
            default:
                return;
        }

        float orientationValues[] = new float[3];
        orientationValues=getOrientation();
        // Pull out the individual values from the array.
        azimuth = orientationValues[0];
        pitch = orientationValues[1];
        roll = orientationValues[2];

        aX=roll;
        aY=pitch;

        //Calculate stable value for roll
        calculatedRoll=(int) Math.max(-100, Math.min(100, -applyNonLinearFunction(-(-defaultRoll+aX)*100)));
        //Calculate stable value for pitch
        calculatedPitch= (int) Math.max(-100, Math.min(100, -applyNonLinearFunction((-defaultPitch+aY)*100)));

        //Calculate stable value for Yaw
        azimuthDeg=(float)Math.toDegrees(azimuth);
        if(Math.abs(azimuthDeg-defaultAzimuthDeg)< 15){
            calculatedYaw=0;
        }
        else{
            calculatedYaw=(int)(azimuthDeg-defaultAzimuthDeg);
        }

        //Make use of one contrl at a time and set other controls as zero
        //When roll, there should be no yaw and pitch
        if(Math.abs(calculatedRoll)>20){

            calculatedYaw=0;
            calculatedPitch=0;
        }
        else{
            calculatedRoll=0;
        }

        //When pitch, there should be no yaw and roll
        if(Math.abs(calculatedPitch)>20){

            calculatedYaw=0;
            calculatedRoll=0;
        }
        else{
            calculatedPitch=0;
        }

        //When yaw, there should be no roll and pitch
        if(Math.abs(calculatedYaw)>15 && (Math.abs(calculatedRoll)<20 || Math.abs(calculatedPitch)<20)){
            calculatedRoll=0;
            calculatedPitch=0;
        }
        else{
            calculatedYaw=0;
        }

        //display values on app
        currentX.setText(String.valueOf(minMax100(calculatedRoll)));
        currentY.setText(String.valueOf(minMax100(calculatedPitch)));
        mTextSensorAzimuth.setText(String.valueOf(minMax100(calculatedYaw)));

        //Display direction
        displayDirection();

    }

    /**
     * Author:Pranjal Singh Rajput
     * Description: Method to calculate the direction
     */
    private void displayDirection(){

        if (azimuth >-0.5 && azimuth<0.5) {
            value_direction.setText("north");
        }
        if (azimuth >0.5 && azimuth<1.0) {
            value_direction.setText("NE");
        }
        if (azimuth >1.0 && azimuth<2.0) {
            value_direction.setText("east");
        }
        if (azimuth >2.0 && azimuth<2.6) {
            value_direction.setText("SE");
        }
        if ((azimuth >2.6 && azimuth<3.10) ||(azimuth >-3.0 && azimuth<-2.6) ) {
            value_direction.setText("south");
        }
        if (azimuth >-2.6 && azimuth<-2.0) {
            value_direction.setText("SW");
        }
        if (azimuth >-2.0 && azimuth<-1.0) {
            value_direction.setText("west");
        }
        if (azimuth >-1.0 && azimuth<-0.5) {
            value_direction.setText("NW");
        }
    }

    State state = new State(), newState = new State();

    /**
     * Author:Pranjal Singh Rajput
     * Description: Set min value to -100 and max value to 100
     */
    private int minMax100(int in) {
        if (in < -100) {
            in = -100;
        } else if (in > 100) {
            in = 100;
        }
        return in;
    }

    /**
     * Author:Pranjal Singh Rajput
     * Description: Method for sending the message
     */
    private void timerExpired() {

        CheckBox c = findViewById(R.id.cbSend);
        if (!c.isChecked()) {
            return;
        }

        Log.d("drone app", "timer expired");

        byte[] data = new byte[7];
        int lift = minMax100(newState.liftKeyboard + seekLift.getProgress());

        int roll = minMax100(newState.rollKeyboard + calculatedRoll);
        int pitch = minMax100(newState.pitchKeyboard + calculatedPitch);
        int yaw = minMax100(newState.yawKeyboard + (calculatedYaw*10)); // joyStickAxis[2]);
        data[0] = i2b(lift);
        data[1] = i2b(roll);
        data[2] = i2b(pitch);
        data[3] = i2b(yaw);
        data[4] = i2b(newState.controllerPYawKeyboard);
        data[5] = i2b(newState.controllerP1RollPitchKeyboard);
        data[6] = i2b(newState.controllerP2RollPitchKeyboard);
        mService.sendMessage(constructMessage(c2b('m'), data));
    }

    /**
     * Author:Pranjal Singh Rajput
     * Description: Apply non-linearity to the roll, pitch and yaw values.
     */
    float applyNonLinearFunction(float f) {
        if (Math.abs(f) < 1.4) {
            return 0;
        } else {
            return Math.signum(f) * (Math.abs(f) - 1.4f);
        }
    }

    /**
     * Author:Pranjal Singh Rajput
     * Description: To calculate the roll, pitch , yaw values using accelerometer and magnetometer data.
     */
    public float[] getOrientation(){
        // Compute the rotation matrix: merges and translates the data

        float[] rotationMatrix = new float[9];
        boolean rotationOK = SensorManager.getRotationMatrix(rotationMatrix,
                null, mAccelerometerData, mMagnetometerData);

        float[] rotationMatrixAdjusted = new float[9];
        switch (mDisplay.getRotation()) {
            case Surface.ROTATION_0:
                rotationMatrixAdjusted = rotationMatrix.clone();
                break;
            case Surface.ROTATION_90:
                SensorManager.remapCoordinateSystem(rotationMatrix,
                        SensorManager.AXIS_Y, SensorManager.AXIS_MINUS_X,
                        rotationMatrixAdjusted);
                break;
            case Surface.ROTATION_180:
                SensorManager.remapCoordinateSystem(rotationMatrix,
                        SensorManager.AXIS_MINUS_X, SensorManager.AXIS_MINUS_Y,
                        rotationMatrixAdjusted);
                break;
            case Surface.ROTATION_270:
                SensorManager.remapCoordinateSystem(rotationMatrix,
                        SensorManager.AXIS_MINUS_Y, SensorManager.AXIS_X,
                        rotationMatrixAdjusted);
                break;
            default:
                rotationMatrixAdjusted = rotationMatrix;
        }

        // Get the orientation of the device (azimuth, pitch, roll) based
        // on the rotation matrix. Output units are radians.
        float orientationValues[] = new float[3];
        if (rotationOK) {
            SensorManager.getOrientation(rotationMatrixAdjusted,
                    orientationValues);
        }
        //--upside down when abs roll > 90--
        if (Math.abs(orientationValues[2]) > Math.PI/2) {
            //--fix, azimuth always to true north, even when device upside down, realistic --
            orientationValues[0] = -orientationValues[0];
            orientationValues[1] = -orientationValues[1];
        }
        return orientationValues;
    }
}
