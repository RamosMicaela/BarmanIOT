package com.example.myapplication;

import android.bluetooth.BluetoothAdapter;
import android.content.Intent;
import android.support.v4.view.ViewPager;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.hardware.SensorManager;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;

import com.example.myapplication.adapters.TragosPagerAdapter;
import com.example.myapplication.model.Trago;

public class TragosActivity extends AppCompatActivity implements SensorEventListener {
    // Variables de sensores
    private SensorManager adminSensores;
    private static final int UMBRAL_SACUDIDA = 200;
    private static final int UMBRAL_ACTUALIZACION = 500;
    private long tiempoUltimaActualizacion;
    private float ultimoX;
    private float ultimoY;
    private float ultimoZ;
    private Boolean isFirstTime = true;
    AlertDialog dialog;

    private TragosPagerAdapter tragosAdapter;
    private ViewPager viewPager;
    private Trago selectedTrago = new Trago();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_tragos);

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage("Elegí otro trago").setTitle("Trago cancelado");
        dialog = builder.create();

        viewPager = (ViewPager) findViewById(R.id.tragos_view_pager);
        tragosAdapter = new TragosPagerAdapter(this);
        viewPager.setAdapter(tragosAdapter);
        adminSensores = (SensorManager) getSystemService(SENSOR_SERVICE);
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
        adminSensores.registerListener(this, adminSensores.getDefaultSensor(Sensor.TYPE_ORIENTATION), SensorManager.SENSOR_DELAY_NORMAL);
    }

    private void pararSensores() {
        adminSensores.unregisterListener(this, adminSensores.getDefaultSensor(Sensor.TYPE_ACCELEROMETER));
        adminSensores.unregisterListener(this, adminSensores.getDefaultSensor(Sensor.TYPE_PROXIMITY));
        adminSensores.unregisterListener(this, adminSensores.getDefaultSensor(Sensor.TYPE_ORIENTATION));
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

            if (velocidad > UMBRAL_SACUDIDA) {
                this.selectedTrago = tragosAdapter.getTrago(viewPager.getCurrentItem());
            }

            this.ultimoX = x;
            this.ultimoY = y;
            this.ultimoZ = z;
        }
    }

    private void getProximity(SensorEvent event) {
        if (!this.isFirstTime && event.values[0] < 7 && this.selectedTrago != null) {
            // Cancelar trago
            dialog.show();
            this.selectedTrago =  null;
        }
        this.isFirstTime = false;
    }

    private void swipeTragos(SensorEvent event) {
        float anguloEnY = (float) (event.values[1] * (180/Math.PI));

        if(anguloEnY >= 60 && anguloEnY <= 100) {
            //swipe a la derecha
            System.out.println("Swipe a la derecha");
        } else if(anguloEnY <= -60 && anguloEnY >= -100) {
            //swipe a la izquierda
            System.out.println("Swipe a la izquierda");
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

                case Sensor.TYPE_ORIENTATION:
                    swipeTragos(event);
                    break;
            }
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {

    }
}
