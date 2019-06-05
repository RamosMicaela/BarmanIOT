package com.example.myapplication;

import android.support.v4.view.ViewPager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.RecyclerView;

import com.example.myapplication.adapters.TragosPagerAdapter;
import com.example.myapplication.model.Trago;

public class TragosActivity extends AppCompatActivity {
    private RecyclerView recyclerView;
    private RecyclerView.Adapter mAdapter;
    private RecyclerView.LayoutManager layoutManager;
    private Trago[] myDataset;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_tragos);

        Trago trago = new Trago("Fernet con Coca", .5f);
        myDataset = new Trago[5];
        myDataset[0] = trago;
        myDataset[1] = trago;
        myDataset[2] = trago;
        myDataset[3] = trago;
        myDataset[4] = trago;


        ViewPager viewPager = (ViewPager) findViewById(R.id.tragos_view_pager);
        viewPager.setAdapter(new TragosPagerAdapter(this));

    }
}
