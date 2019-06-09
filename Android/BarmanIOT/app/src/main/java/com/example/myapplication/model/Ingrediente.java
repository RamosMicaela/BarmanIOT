package com.example.myapplication.model;

public class Ingrediente {
    private String nombre;
    private float cantidad;
    private String unidad;


    public Ingrediente(String nombre, float cantidad, String unidad) {
        this.nombre = nombre;
        this.cantidad = cantidad;
        this.unidad = unidad;
    }

    public String getNombre() {
        return nombre;
    }

    public float getCantidad() {
        return cantidad;
    }

    public String getUnidad() {
        return unidad;
    }
}
