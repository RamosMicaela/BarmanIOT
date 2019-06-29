package com.example.myapplication.adapters;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.v4.view.PagerAdapter;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.example.myapplication.MainActivity;
import com.example.myapplication.R;
import com.example.myapplication.model.Trago;
import com.example.myapplication.model.Ingrediente;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;

public class TragosPagerAdapter extends PagerAdapter {
    private Context mContext;
    private ArrayList<Trago> mTragos;
    private ArrayList<Ingrediente> mIngredientes;
    private ArrayAdapter<String> ingNombreAdapter;
    private ArrayAdapter<String> ingCantidadAdapter;
    private ArrayAdapter<String> ingUnidadAdapter;
    private ArrayList<String> sIngredientes;
    private ArrayList<String> sIngredientesCantidad;
    private ArrayList<String> sIngredientesUnidad;
    private int actualPosition;

    public TragosPagerAdapter(Context context) {
        mContext = context;
        mTragos = new ArrayList<>();
        mIngredientes = new ArrayList<>();

        mIngredientes.add(new Ingrediente("Agua", 100, "%"));
        Trago trago = new Trago("Sucas Lecchi", 0.0f, R.drawable.ic_water, mIngredientes);
        mTragos.add(trago);
        mIngredientes.clear();

        mIngredientes.add(new Ingrediente("Fernet", 20, "%"));
        mIngredientes.add(new Ingrediente("Coca Cola", 60, "%"));
        trago = new Trago("Fernet con Coca", .5f, R.drawable.ic_fernet, mIngredientes);
        mTragos.add(trago);
        mIngredientes.clear();

        mIngredientes.add(new Ingrediente("Vodka", 30, "%"));
        mIngredientes.add(new Ingrediente("Jugo de naranja", 70, "%"));
        trago = new Trago("Destornillador", .3f, R.drawable.ic_destornillador, mIngredientes);
        mTragos.add(trago);
        mIngredientes.clear();

        mIngredientes.add(new Ingrediente("Gancia", 80, "%"));
        mIngredientes.add(new Ingrediente("Sprite", 20, "%"));
        mIngredientes.add(new Ingrediente("Azúcar", 0, "C/N"));
        trago = new Trago("Gancia con Sprite", .4f, R.drawable.ic_gancia, mIngredientes);
        mTragos.add(trago);
        mIngredientes.clear();

        mIngredientes.add(new Ingrediente("Coca Cola", 20, "%"));
        mIngredientes.add(new Ingrediente("Ron dorado", 20, "%"));
        mIngredientes.add(new Ingrediente("Jugo", 0.5, "Lima"));
        trago = new Trago("Cuba Libre", .2f, R.drawable.ic_wisky, mIngredientes);
        mTragos.add(trago);
        mIngredientes.clear();

        mIngredientes.add(new Ingrediente("Ron blanco", 2, "oz"));
        mIngredientes.add(new Ingrediente("Jarabe de piña", 2, "oz"));
        mIngredientes.add(new Ingrediente("Crema de coco", 1, "oz"));
        mIngredientes.add(new Ingrediente("Agua mineral", 2, "oz"));
        mIngredientes.add(new Ingrediente("Licor de banana", 1, ""));
        trago = new Trago("Banana Mama", .1f, R.drawable.ic_daikiri, mIngredientes);
        mTragos.add(trago);
        mIngredientes.clear();
    }

    @Override
    public int getCount() {
        return mTragos.size();
    }

    @Override
    public boolean isViewFromObject(@NonNull View view, @NonNull Object object) {
        return view == object;
    }

    @NonNull
    @Override
    public Object instantiateItem(@NonNull ViewGroup collection, int position) {
        sIngredientes = new ArrayList<>();
        sIngredientesCantidad = new ArrayList<>();
        sIngredientesUnidad = new ArrayList<>();

        LayoutInflater inflater = LayoutInflater.from(mContext);
        ViewGroup layout = (ViewGroup) inflater.inflate(R.layout.page_trago, collection, false);
        ((TextView)layout.findViewById(R.id.nombreTextView)).setText(mTragos.get(position).getNombre());
        ((ImageView)layout.findViewById(R.id.tragoImage)).setImageResource(mTragos.get(position).getmIcon());
        RecyclerView recyclerView = (RecyclerView)layout.findViewById(R.id.ingredientesRecyclerView);
        ArrayList<Ingrediente> ingredientes = mTragos.get(position).getIngredientes();

        recyclerView.setHasFixedSize(true);
        LinearLayoutManager llm = new LinearLayoutManager(layout.getContext());
        recyclerView.setLayoutManager(llm);

        for (Ingrediente ingrediente : ingredientes) {
            String nombre = ingrediente.getNombre();
            String cantidad = String.valueOf(ingrediente.getCantidad());
            String unidad = ingrediente.getUnidad();
            sIngredientes.add(nombre);
            sIngredientesCantidad.add(cantidad);
            sIngredientesUnidad.add(unidad);
        }

        IngredientesRecyclerViewAdapter adapter = new IngredientesRecyclerViewAdapter(ingredientes);
        recyclerView.setAdapter(adapter);

        collection.addView(layout);
        actualPosition = position;
        return layout;
    }

    @Override
    public void destroyItem(@NonNull ViewGroup collection, int position, @NonNull Object view) {
        collection.removeView((View) view);
    }


    @Override
    public CharSequence getPageTitle(int position) {
        return mTragos.get(position).getNombre();
    }

    public Trago getTrago(int position) {
        return mTragos.get(position);
    }
}