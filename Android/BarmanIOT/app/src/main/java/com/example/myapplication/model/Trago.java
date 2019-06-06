package com.example.myapplication.model;

import android.graphics.drawable.Drawable;

public class Trago {
    private String mNombre;
    private float mGraduacion;
    private int mIcon;

    public Trago(String mNombre, float mGraduacion, int mIcon) {
        this.mNombre = mNombre;
        this.mGraduacion = mGraduacion;
        this.mIcon = mIcon;
    }

    public String getNombre() {
        return mNombre;
    }

    public float getGraduacion() {
        return mGraduacion;
    }

    public int getmIcon() { return mIcon; }

}
