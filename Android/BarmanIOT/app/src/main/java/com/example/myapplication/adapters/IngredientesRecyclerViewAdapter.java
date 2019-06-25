package com.example.myapplication.adapters;

import android.support.annotation.NonNull;
import android.support.v7.widget.CardView;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.example.myapplication.R;
import com.example.myapplication.model.Ingrediente;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;


public class IngredientesRecyclerViewAdapter extends RecyclerView.Adapter<IngredientesRecyclerViewAdapter.IngredienteViewHolder> {
    public static class IngredienteViewHolder extends RecyclerView.ViewHolder {
        CardView cv;
        TextView ingredienteNombre;
        TextView ingredienteCantidad;
        TextView ingredienteUnidad;

        IngredienteViewHolder(View itemView) {
            super(itemView);
//            cv = (CardView)itemView.findViewById(R.id.cv);
            ingredienteNombre = (TextView)itemView.findViewById(R.id.nombreIngr);
            ingredienteCantidad = (TextView)itemView.findViewById(R.id.cantidadIngr);
            ingredienteUnidad = (TextView) itemView.findViewById(R.id.unidadIngr);
        }
    }
    private ArrayList<Ingrediente> ingredientes;

    IngredientesRecyclerViewAdapter(ArrayList<Ingrediente> ingredientes){
        this.ingredientes = ingredientes;
    }

    @NonNull
    @Override
    public IngredienteViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
        View v = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.list_item_ingrediente, viewGroup, false);
        return new IngredienteViewHolder(v);
    }

    @Override
    public void onBindViewHolder(@NonNull IngredienteViewHolder ingredienteViewHolder, int i) {
        ingredienteViewHolder.ingredienteNombre.setText(ingredientes.get(i).getNombre());
        ingredienteViewHolder.ingredienteCantidad.setText(String.format(Locale.getDefault(),"%.2f",ingredientes.get(i).getCantidad()));
        ingredienteViewHolder.ingredienteUnidad.setText(ingredientes.get(i).getUnidad());
    }

    @Override
    public int getItemCount() {
        return ingredientes.size();
    }

    @Override
    public void onAttachedToRecyclerView(@NonNull RecyclerView recyclerView) {
        super.onAttachedToRecyclerView(recyclerView);
    }
}
