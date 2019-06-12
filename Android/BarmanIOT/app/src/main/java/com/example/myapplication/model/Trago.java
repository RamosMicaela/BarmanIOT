package com.example.myapplication.model;

import android.graphics.drawable.Drawable;
import android.widget.ArrayAdapter;

import java.util.ArrayList;

public class Trago {
    private String mNombre;
    private float mGraduacion;
    private int mIcon;
    private ArrayList<Ingrediente> mIngredientes;

    public Trago(String mNombre, float mGraduacion, int mIcon, ArrayList<Ingrediente> ingredientes) {
        this.mIngredientes = new ArrayList<>();
        this.mNombre = mNombre;
        this.mGraduacion = mGraduacion;
        this.mIcon = mIcon;
        this.mIngredientes.addAll(ingredientes);
    }

    public String getNombre() {
        return mNombre;
    }

    public float getGraduacion() {
        return mGraduacion;
    }

    public int getmIcon() { return mIcon; }

    public ArrayList<Ingrediente> getIngredientes() { return mIngredientes; }
}
