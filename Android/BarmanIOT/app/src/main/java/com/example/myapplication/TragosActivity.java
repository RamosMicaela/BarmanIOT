package com.example.myapplication;

import android.os.Handler;
import android.os.Message;
import android.support.v4.view.ViewPager;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.hardware.SensorManager;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.view.View;
import android.widget.Toast;
import com.example.myapplication.adapters.TragosPagerAdapter;
import com.example.myapplication.model.Trago;
import java.lang.ref.WeakReference;

public class TragosActivity extends AppCompatActivity implements SensorEventListener {
    // Variables de sensores
    private SensorManager adminSensores;
    private static final int UMBRAL_SACUDIDA = 250;
    private static final int UMBRAL_ACTUALIZACION = 500;
    private long tiempoUltimaActualizacion;
    private float ultimoX;
    private float ultimoY;
    private float ultimoZ;
    private Boolean isFirstTime = true, hasSwiped = false;

    AlertDialog dialog;
    AlertDialog dialogFinBebida;

    private TragosPagerAdapter tragosAdapter;
    private ViewPager viewPager;
    private Trago selectedTrago;
    private static final int MAX_CHARS_FOR_NAME = 16;

    private BluetoothData btDatos;
    private BluetoothWrite write;
    private BluetoothRead read;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_tragos);

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage("Elegí otro trago").setTitle("Trago cancelado");
        dialog = builder.create();

        AlertDialog.Builder builderFinBebida = new AlertDialog.Builder(this);
        builderFinBebida.setMessage("Elegí otro trago").setTitle("Trago Finalizado");
        dialogFinBebida = builderFinBebida.create();

        viewPager = (ViewPager) findViewById(R.id.tragos_view_pager);
        tragosAdapter = new TragosPagerAdapter(this);
        viewPager.setAdapter(tragosAdapter);
        adminSensores = (SensorManager) getSystemService(SENSOR_SERVICE);

        btDatos = BluetoothData.getInstance();
        write = BluetoothWrite.getInstance(btDatos.getSocket(),this);

        Handler handler = new MyHandler(this);
        read = new BluetoothRead(handler);
        read.start();
        inicializarSensores();
    }

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
        super.onDestroy();
    }

    @Override
    protected void onStop() {
        pararSensores();
        super.onStop();
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
                int currentItem = viewPager.getCurrentItem();
                this.selectedTrago = tragosAdapter.getTrago(currentItem);
                String trago = buildStringToSend();
                write.write(trago);
            }

            this.ultimoX = x;
            this.ultimoY = y;
            this.ultimoZ = z;
        }
    }

    private void getProximity(SensorEvent event) {
        if (!this.isFirstTime && event.values[0] < 3 && this.selectedTrago != null) {
            // Cancelar trago
            write.write("2");
            this.selectedTrago =  null;
            dialog.show();
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


    public void showToast(String message) {
        Toast.makeText(this, message, Toast.LENGTH_SHORT).show();
    }

    private String buildStringToSend() {
        String name = completeString(this.selectedTrago.getNombre(), MAX_CHARS_FOR_NAME);
        String ingredientes = "";

        // Esto es complmetamente contra mi voluntad
        for(int i = 0; i < this.selectedTrago.getIngredientes().size(); i++) {
            if (this.selectedTrago.getIngredientes().get(i).getUnidad().equals("%")){
                double cantidad = this.selectedTrago.getIngredientes().get(i).getCantidad() / 100;
                String ingName = completeString(this.selectedTrago.getIngredientes().get(i).getNombre(), MAX_CHARS_FOR_NAME);
                String cant = cantidad < 1 && cantidad >= 0.1 ? '0' + String.valueOf((int) (100 * cantidad)) : cantidad < 0.1 ? "00" + String.valueOf((int) (100 * cantidad)) :
                        String.valueOf((int) (100 * cantidad));

                ingredientes += ingName + cant;
            }
        }

        return "0" + name + ingredientes;
    }

    private String completeString(String string, int cantChars) {
        // Y esto tambien
            int cantActual = string.length();

            while( cantActual < cantChars) {
                string += '*';
                cantActual = string.length();
            }

        return string;
    }

    public void onClickRecalibrate(View v) {
        AlertDialog dialogRecalibrate;

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage("Recalibra tu vaso").setTitle("Recalibrar");
        dialogRecalibrate = builder.create();

        if(this.selectedTrago == null) {
            write.write("1");
            dialogRecalibrate.show();
        } else {
            showToast("Ya tenes un trago seleccionado!");
        }
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

    private static class MyHandler extends Handler {
        private final WeakReference<TragosActivity> myClassWeakReference;

        private MyHandler(TragosActivity instancia) {
            myClassWeakReference = new WeakReference<>(instancia);
        }

        @Override
        public void handleMessage(Message msg) {
            TragosActivity myClass = myClassWeakReference.get();
            String key = "recibido";
            Bundle bundle = msg.getData();
            String cad = bundle.getString(key);
            if (cad != null && cad.equals("100"))
            {
                myClass.dialogFinBebida.show();
                myClass.selectedTrago =  null;
            }
        }
    }
}
