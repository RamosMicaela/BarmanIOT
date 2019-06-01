package com.example.myapplication.model;

public class Trago {
    private String mNombre;
    private float mGraduacion;

    public Trago(String mNombre, float mGraduacion) {
        this.mNombre = mNombre;
        this.mGraduacion = mGraduacion;
    }

    public String getNombre() {
        return mNombre;
    }

    public float getGraduacion() {
        return mGraduacion;
    }

}
