package com.example.myapplication;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;

import com.example.myapplication.adapters.TragosAdapter;
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

        recyclerView = (RecyclerView) findViewById(R.id.tragos_recycler_view);

        // use this setting to improve performance if you know that changes
        // in content do not change the layout size of the RecyclerView
        recyclerView.setHasFixedSize(true);

        // use a linear layout manager
        LinearLayoutManager llm =  new LinearLayoutManager(this);
        llm.setOrientation(LinearLayoutManager.VERTICAL);
        recyclerView.setLayoutManager(layoutManager);

        // specify an adapter (see also next example)
        mAdapter = new TragosAdapter(myDataset);
        recyclerView.setAdapter(mAdapter);


    }
}
