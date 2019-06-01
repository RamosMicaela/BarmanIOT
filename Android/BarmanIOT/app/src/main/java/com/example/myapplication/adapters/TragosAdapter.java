package com.example.myapplication.adapters;

import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.widget.TextView;

import com.example.myapplication.R;
import com.example.myapplication.model.Trago;

public class TragosAdapter extends RecyclerView.Adapter<TragosAdapter.TragoViewHolder> {
    private Trago[] mDataset;

    // Provide a reference to the views for each data item
    // Complex data items may need more than one view per item, and
    // you provide access to all the views for a data item in a view holder
    public static class TragoViewHolder extends RecyclerView.ViewHolder {
        // each data item is just a string in this case
        public TextView textView;

        public TragoViewHolder(TextView v) {
            super(v);
            textView = v;
        }
    }

    public TragosAdapter(Trago[] dataset) {
        mDataset = dataset;
    }

    @NonNull
    @Override
    public TragoViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        // create a new view
        TextView v = (TextView) LayoutInflater.from(parent.getContext())
                .inflate(R.layout.activity_tragos, parent, false);
        TragoViewHolder vh = new TragoViewHolder(v);
        return vh;

    }

    @Override
    public void onBindViewHolder(@NonNull TragoViewHolder viewHolder, int position) {
        // - get element from your dataset at this position
        // - replace the contents of the view with that element
        viewHolder.textView.setText(mDataset[position].getNombre());

    }

    @Override
    public int getItemCount() {
        return mDataset.length;
    }
}
