package com.example.myapplication;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Handler;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v4.view.ViewPager;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.hardware.SensorManager;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.example.myapplication.adapters.TragosPagerAdapter;
import com.example.myapplication.model.Trago;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.UUID;

public class TragosActivity extends AppCompatActivity implements SensorEventListener {
    // Variables de sensores
    private SensorManager adminSensores;
    private static final int UMBRAL_SACUDIDA = 300;
    private static final int UMBRAL_ACTUALIZACION = 500;
    private long tiempoUltimaActualizacion;
    private float ultimoX;
    private float ultimoY;
    private float ultimoZ;
    private Boolean isFirstTime = true, hasSwiped = false;
    float easing = 0.01F;

    AlertDialog dialog;

    private TragosPagerAdapter tragosAdapter;
    private ViewPager viewPager;
    private Trago selectedTrago;
    private static final int MAX_CHARS_FOR_NAME = 16;


    BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
    private static final int REQUEST_ENABLE_BT = 1;
    private BluetoothDevice btDevice;

    public static final int MULTIPLE_PERMISSIONS = 10; // code you want.

    //se crea un array de String con los permisos a solicitar en tiempo de ejecucion
    //Esto se debe realizar a partir de Android 6.0, ya que con verdiones anteriores
    //con solo solicitarlos en el Manifest es suficiente
    String[] permissions= new String[]{
            Manifest.permission.BLUETOOTH,
            Manifest.permission.BLUETOOTH_ADMIN,
            Manifest.permission.ACCESS_COARSE_LOCATION,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_PHONE_STATE,
            Manifest.permission.READ_EXTERNAL_STORAGE
    };

    private BluetoothSocket btSocket = null;
    private ConnectedThread mConnectedThread;
    Handler bluetoothIn;
    final int handlerState = 0;

    Button selectButton;

    // SPP UUID service  - Funciona en la mayoria de los dispositivos
    private static final UUID BTMODULEUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_tragos);

        if (checkPermissions()) {
            enableComponent();
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage("Elegí otro trago").setTitle("Trago cancelado");
        dialog = builder.create();

        viewPager = (ViewPager) findViewById(R.id.tragos_view_pager);
        tragosAdapter = new TragosPagerAdapter(this);
        viewPager.setAdapter(tragosAdapter);

        adminSensores = (SensorManager) getSystemService(SENSOR_SERVICE);
        inicializarSensores();
    }

   /*
   Para probar funcion que completa los strings
   public void onClickSelect (View v) {
        this.selectedTrago = new Trago();
        this.selectedTrago = tragosAdapter.getTrago(viewPager.getCurrentItem());
        String trago = buildStringToSend();

        System.out.println("Este es el string que queda");
        System.out.println(trago);
    }
    */

    @Override
    protected void onRestart() {
        inicializarSensores();
        super.onRestart();
    }

    @Override
    protected void onResume() {
        super.onResume();
        inicializarSensores();
    }

    @Override
    protected void onPause() {
        pararSensores();
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        pararSensores();
        unregisterReceiver(bReciever);
        unregisterReceiver(mPairReceiver);
        super.onDestroy();
    }

    @Override
    protected void onStop() {
        pararSensores();
        super.onStop();
    }

    protected  void enableComponent() {
        //se determina si existe bluethoot en el celular
        if (mBluetoothAdapter == null) {
            //si el celular no soporta bluethoot
            showToast("El dispositivo no soporta bluetooth");
        } else {
            //si el celular soporta bluethoot, se determina si esta activado el bluethoot
            if (mBluetoothAdapter.isEnabled()) {
                if (mBluetoothAdapter.isDiscovering()) {
                    // El Bluetooth ya está en modo discover, lo cancelamos para iniciarlo de nuevo
                    mBluetoothAdapter.cancelDiscovery();
                }
                mBluetoothAdapter.startDiscovery();
            } else {
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
            }
        }

        //se definen un broadcastReceiver que captura el broadcast del SO cuando captura los siguientes eventos:
        IntentFilter filter = new IntentFilter();

        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);  //Cambia el estado del Bluethoot (Acrtivado /Desactivado)
        filter.addAction(BluetoothDevice.ACTION_FOUND); //Se encuentra un dispositivo bluethoot al realizar una busqueda
        filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_STARTED); //Cuando se comienza una busqueda de bluethoot
        filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED); //cuando la busqueda de bluethoot finaliza
        filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED); //Cuando se empareja o desempareja el bluethoot

        //se registra el handler que captura los broadcast anterirmente mencionados.
        registerReceiver(mPairReceiver, filter);
        registerReceiver(bReciever, filter);
    }

    private void inicializarSensores() {
        adminSensores.registerListener(this, adminSensores.getDefaultSensor(Sensor.TYPE_ACCELEROMETER), SensorManager.SENSOR_DELAY_NORMAL);
        adminSensores.registerListener(this, adminSensores.getDefaultSensor(Sensor.TYPE_PROXIMITY), SensorManager.SENSOR_DELAY_NORMAL);
        adminSensores.registerListener(this, adminSensores.getDefaultSensor(Sensor.TYPE_GRAVITY), SensorManager.SENSOR_DELAY_NORMAL);
    }

    private void pararSensores() {
        adminSensores.unregisterListener(this, adminSensores.getDefaultSensor(Sensor.TYPE_ACCELEROMETER));
        adminSensores.unregisterListener(this, adminSensores.getDefaultSensor(Sensor.TYPE_PROXIMITY));
        adminSensores.unregisterListener(this, adminSensores.getDefaultSensor(Sensor.TYPE_GRAVITY));
    }

    private void getShake(SensorEvent event) {
        float x, y, z;
        double aceleracionAnterior, aceleracionActual, velocidad;
        long tiempoActual = System.currentTimeMillis();
        long diferenciaDeTiempo = tiempoActual - tiempoUltimaActualizacion;

        if (diferenciaDeTiempo > UMBRAL_ACTUALIZACION) {

            tiempoUltimaActualizacion = tiempoActual;

            x = event.values[0];
            y = event.values[1];
            z = event.values[2];

            aceleracionActual = x + y + z;
            aceleracionAnterior = this.ultimoX + this.ultimoY + this.ultimoZ;

            velocidad = Math.abs(aceleracionActual - aceleracionAnterior) / diferenciaDeTiempo * 10000;

            if (velocidad > UMBRAL_SACUDIDA && this.selectedTrago == null) {
                this.selectedTrago = new Trago();
                this.selectedTrago = tragosAdapter.getTrago(viewPager.getCurrentItem());
                String trago = buildStringToSend();
                mConnectedThread.write(trago);
            }

            this.ultimoX = x;
            this.ultimoY = y;
            this.ultimoZ = z;
        }
    }

    private void getProximity(SensorEvent event) {
        if (!this.isFirstTime && event.values[0] < 7 && this.selectedTrago != null) {
            // Cancelar trago
            mConnectedThread.write("2");
            dialog.show();
            this.selectedTrago =  null;
        }
        this.isFirstTime = false;
    }

    private void swipeTragos(SensorEvent event) {
        float x = event.values[0]; //Gravedad aplicada en el eje x del dispositivo

        if(x <= -4) {
            //swipe a la derecha
            if(!hasSwiped) {
                viewPager.setCurrentItem(viewPager.getCurrentItem() + 1, true);
                hasSwiped = true;
            }
        } else if(x >= 4) {
            //swipe a la izquierda
            if(!hasSwiped) {
                viewPager.setCurrentItem(viewPager.getCurrentItem() - 1, true);
                hasSwiped = true;
            }
        }else if ( Math.abs(x) <= 4.0){
            hasSwiped = false;
        }

    }


    private void showToast(String message) {
        Toast.makeText(this, message, Toast.LENGTH_SHORT).show();
    }

    private String buildStringToSend() {
        String name = completeString(this.selectedTrago.getNombre(), MAX_CHARS_FOR_NAME);
        String ingredientes = "";

        // Esto es complmetamente contra mi voluntad
        for(int i = 0; i < this.selectedTrago.getIngredientes().size(); i++) {
            double cantidad = this.selectedTrago.getIngredientes().get(i).getCantidad() / 100;

            String ingName = completeString(this.selectedTrago.getIngredientes().get(i).getNombre(), MAX_CHARS_FOR_NAME);
            String cant = cantidad < 1  && cantidad >= 0.1? '0' + String.valueOf((int)(100 * cantidad)) : cantidad < 0.1 ? "00" + String.valueOf((int)(100 * cantidad)) :
                    String.valueOf((int)(100 * cantidad));

            ingredientes += ingName + cant;
        }

        return "0" + name + ingredientes;
    }

    private String completeString(String string, int cantChars) {
        // Y esto tambien
            int cantActual = string.length();

            while( cantActual < cantChars) {
                string += ' ';
                cantActual = string.length();
            }

        return string;
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        synchronized (this) {
            int tipoEvento = event.sensor.getType();
            switch(tipoEvento) {

                case Sensor.TYPE_ACCELEROMETER:
                    getShake(event);
                    break;

                case Sensor.TYPE_PROXIMITY:
                    getProximity(event);
                    break;

                case Sensor.TYPE_GRAVITY:
                    swipeTragos(event);
                    break;

            }
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {

    }

    public void  establishBluetoothconnection() {
        if (btDevice.getBondState() == BluetoothDevice.BOND_BONDED) {
            //Si esta emparejado,quiere decir que se selecciono desemparjar y entonces se le desempareja
            unpairDevice(btDevice);
        } else {
            //Si no esta emparejado,quiere decir que se selecciono emparjar y entonces se le empareja
            showToast("Emparejando");
            pairDevice(btDevice);

        }
    }

    private void pairDevice(BluetoothDevice device) {
        try {
            Method method = device.getClass().getMethod("createBond", (Class[]) null);
            method.invoke(device, (Object[]) null);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void unpairDevice(BluetoothDevice device) {
        try {
            Method method = device.getClass().getMethod("removeBond", (Class[]) null);
            method.invoke(device, (Object[]) null);

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private final BroadcastReceiver bReciever = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (BluetoothDevice.ACTION_FOUND.equals(action)) {
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                // Encuentra nuestra placa bluetooth
                if(device.getAddress().equals("00:13:EF:00:B8:70")) {
                    btDevice = device;
                    establishBluetoothconnection();
                    showToast("Bluetooth encontrado");
                }
            }
        }
    };

    //Handler que captura los brodacast que emite el SO al ocurrir los eventos del bluethoot
    private final BroadcastReceiver mPairReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {

            //Atraves del Intent obtengo el evento de Bluetooth que informo el broadcast del SO
            String action = intent.getAction();

            //si el SO detecto un emparejamiento o desemparjamiento de buletooth
            if (BluetoothDevice.ACTION_BOND_STATE_CHANGED.equals(action)) {
                //Obtengo los parametros, aplicando un Bundle, que me indica el estado del Bluethoot
                final int state = intent.getIntExtra(BluetoothDevice.EXTRA_BOND_STATE, BluetoothDevice.ERROR);
                final int prevState = intent.getIntExtra(BluetoothDevice.EXTRA_PREVIOUS_BOND_STATE, BluetoothDevice.ERROR);

                //se analiza si se puedo emparejar o no
                if (state == BluetoothDevice.BOND_BONDED && prevState == BluetoothDevice.BOND_BONDING) {
                    //Si se detecto que se puedo emparejar el bluethoot
                    showToast("Emparejado");
                    try {
                        btSocket = createBluetoothSocket(btDevice);
                    } catch (IOException e) {
                        showToast( "La creacción del Socket fallo");
                    }
                    // Establish the Bluetooth socket connection.
                    try {
                        btSocket.connect();
                    }
                    catch (IOException e) {
                        try {
                            btSocket.close();
                        } catch (IOException e2) {
                            //insert code to deal with this
                        }
                    }

                    //Una establecida la conexion con el Hc06 se crea el hilo secundario, el cual va a recibir
                    // los datos de Arduino atraves del bluethoot
                    mConnectedThread = new ConnectedThread(btSocket);
                    mConnectedThread.start();
                }  //si se detrecto un desaemparejamiento
                else if (state == BluetoothDevice.BOND_NONE && prevState == BluetoothDevice.BOND_BONDED) {
                    showToast("No emparejado");
                }
            }
        }
    };

    private BluetoothSocket createBluetoothSocket(BluetoothDevice device) throws IOException {
        return  device.createRfcommSocketToServiceRecord(BTMODULEUUID);
    }

    //Metodo que chequea si estan habilitados los permisos
    private  boolean checkPermissions() {
        int result;
        List<String> listPermissionsNeeded = new ArrayList<>();

        //Se chequea si la version de Android es menor a la 6
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            return true;
        }

        for (String p:permissions) {
            result = ContextCompat.checkSelfPermission(this,p);
            if (result != PackageManager.PERMISSION_GRANTED) {
                listPermissionsNeeded.add(p);
            }
        }
        if (!listPermissionsNeeded.isEmpty()) {
            ActivityCompat.requestPermissions(this, listPermissionsNeeded.toArray(new String[listPermissionsNeeded.size()]),MULTIPLE_PERMISSIONS );
            return false;
        }
        return true;
    }

    //******************************************** Hilo secundario del Activity **************************************
    //*************************************** recibe los datos enviados por el HC06 **********************************

    private class ConnectedThread extends Thread
    {
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        //Constructor de la clase del hilo secundario
        public ConnectedThread(BluetoothSocket socket)
        {
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            try
            {
                //Create I/O streams for connection
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) { }

            mmInStream = tmpIn;
            mmOutStream = tmpOut;
        }

        //metodo run del hilo, que va a entrar en una espera activa para recibir los msjs del HC05
        public void run()
        {
            byte[] buffer = new byte[256];
            int bytes;

            //el hilo secundario se queda esperando mensajes del HC05
            while (true)
            {
                try
                {
                    //se leen los datos del Bluethoot
                    bytes = mmInStream.read(buffer);
                    String readMessage = new String(buffer, 0, bytes);

                    //se muestran en el layout de la activity, utilizando el handler del hilo
                    // principal antes mencionado
                    bluetoothIn.obtainMessage(handlerState, bytes, -1, readMessage).sendToTarget();
                } catch (IOException e) {
                    break;
                }
            }
        }


        //write method
        public void write(String input) {
            byte[] msgBuffer = input.getBytes();           //converts entered String into bytes
            try {
                mmOutStream.write(msgBuffer);                //write bytes over BT connection via outstream
            } catch (IOException e) {
                //if you cannot write, close the application
                showToast("La conexion fallo");
                finish();

            }
        }
    }
}
