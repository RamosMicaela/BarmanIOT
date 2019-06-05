package com.example.myapplication.adapters;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.v4.view.PagerAdapter;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.example.myapplication.R;
import com.example.myapplication.model.Trago;

import java.util.Locale;

public class TragosPagerAdapter extends PagerAdapter {
    private Context mContext;
    private Trago[] mTragos;
    public TragosPagerAdapter(Context context) {
        mContext = context;
        Trago trago = new Trago("Fernet con Coca", .5f);
        mTragos = new Trago[5];
        mTragos[0] = trago;
        trago = new Trago("Gancia con Sprite", .4f);
        mTragos[1] = trago;
        trago = new Trago("Destornillador", .3f);
        mTragos[2] = trago;
        trago = new Trago("Cuba Libre", .2f);
        mTragos[3] = trago;
        trago = new Trago("Banana Mama", .1f);
        mTragos[4] = trago;
    }

    @Override
    public int getCount() {
        return mTragos.length;
    }

    @Override
    public boolean isViewFromObject(@NonNull View view, @NonNull Object object) {
        return view == object;
    }



    @Override
    public Object instantiateItem(ViewGroup collection, int position) {
        LayoutInflater inflater = LayoutInflater.from(mContext);
        ViewGroup layout = (ViewGroup) inflater.inflate(R.layout.page_trago, collection, false);
        ((TextView)layout.findViewById(R.id.nombreTextView)).setText(mTragos[position].getNombre());
        ((TextView)layout.findViewById(R.id.graduacionTextView)).setText(String.format(Locale.getDefault(),"%.2f",mTragos[position].getGraduacion()));
        collection.addView(layout);
        return layout;
    }

    @Override
    public void destroyItem(ViewGroup collection, int position, Object view) {
        collection.removeView((View) view);
    }


    @Override
    public CharSequence getPageTitle(int position) {
        return mTragos[position].getNombre();
    }

}