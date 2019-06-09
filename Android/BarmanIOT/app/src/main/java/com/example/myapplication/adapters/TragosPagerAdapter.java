package com.example.myapplication.adapters;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.v4.view.PagerAdapter;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import com.example.myapplication.R;
import com.example.myapplication.model.Trago;

import java.util.ArrayList;
import java.util.Locale;

public class TragosPagerAdapter extends PagerAdapter {
    private Context mContext;
    private ArrayList<Trago> mTragos;
    public TragosPagerAdapter(Context context) {
        mContext = context;
        mTragos = new ArrayList<>();

        Trago trago = new Trago("Sucas Lecchi", 0.0f, R.drawable.ic_water);
        mTragos.add(trago);

        trago = new Trago("Fernet con Coca", .5f, R.drawable.ic_fernet);
        mTragos.add(trago);

        trago = new Trago("Destornillador", .3f, R.drawable.ic_destornillador);
        mTragos.add(trago);

        trago = new Trago("Gancia con Sprite", .4f, R.drawable.ic_gancia);
        mTragos.add(trago);

        trago = new Trago("Cuba Libre", .2f, R.drawable.ic_wisky);
        mTragos.add(trago);

        trago = new Trago("Banana Mama", .1f, R.drawable.ic_daikiri);
        mTragos.add(trago);
    }

    @Override
    public int getCount() {
        return mTragos.size();
    }

    @Override
    public boolean isViewFromObject(@NonNull View view, @NonNull Object object) {
        return view == object;
    }



    @Override
    public Object instantiateItem(ViewGroup collection, int position) {
        LayoutInflater inflater = LayoutInflater.from(mContext);
        ViewGroup layout = (ViewGroup) inflater.inflate(R.layout.page_trago, collection, false);

        ((TextView)layout.findViewById(R.id.nombreTextView)).setText(mTragos.get(position).getNombre());
        ((TextView)layout.findViewById(R.id.graduacionTextView)).setText(String.format(Locale.getDefault(),"%.2f",mTragos.get(position).getGraduacion()));
        ((ImageView)layout.findViewById(R.id.tragoImage)).setImageResource(mTragos.get(position).getmIcon());
        collection.addView(layout);
        return layout;
    }

    @Override
    public void destroyItem(ViewGroup collection, int position, Object view) {
        collection.removeView((View) view);
    }


    @Override
    public CharSequence getPageTitle(int position) {
        return mTragos.get(position).getNombre();
    }

}