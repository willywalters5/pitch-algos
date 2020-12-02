package com.ece420.lab4;
import com.ece420.lab4.*;
import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.GridLabelRenderer;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.LineGraphSeries;
import com.jjoe64.graphview.series.PointsGraphSeries;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.media.MediaPlayer;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.support.annotation.Nullable;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Toast;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;
import java.util.Timer;
import java.util.TimerTask;

public class PrerecordActivity extends Activity {
    CheckBox mCEP,mPPROC,mSIFT,mAUTOC;
    //RadioButton mChild,mFemale,mMale;
    RadioButton radioButton;
    RadioGroup radioGroup;
    Button clear;
    MediaPlayer audioChild,audioFemale,audioMale;
    GraphView graph;

    // Static Values
    private static final int AUDIO_ECHO_REQUEST = 0;
    private static final int FRAME_SIZE = 2048;
    private static final int radius=5;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_prerecord);

        mCEP=(CheckBox)findViewById(R.id.CEP);
        mPPROC=(CheckBox)findViewById(R.id.PPROC);
        mSIFT=(CheckBox)findViewById(R.id.SIFT);
        mAUTOC=(CheckBox)findViewById(R.id.AUTOC);
        clear=(Button)findViewById(R.id.clear);

//        mChild=(RadioButton)findViewById(R.id.record_c);
//        mFemale=(RadioButton)findViewById(R.id.record_f);
//        mMale=(RadioButton)findViewById(R.id.record_m);

        radioGroup=(RadioGroup)findViewById(R.id.radio_group);
        audioChild=MediaPlayer.create(PrerecordActivity.this,R.raw.c_247);
        audioFemale=MediaPlayer.create(PrerecordActivity.this,R.raw.f_185);
        audioMale=MediaPlayer.create(PrerecordActivity.this,R.raw.m_82);
        graph = (GraphView) findViewById(R.id.graph);
        GridLabelRenderer gridLabel = graph.getGridLabelRenderer();
        gridLabel.setHorizontalAxisTitle("Time (s)");
        gridLabel.setVerticalAxisTitle("Pitch (Hz)");
        graph.getViewport().setMinX(-0.1);
        graph.getViewport().setMaxX(3.5);
        graph.getViewport().setMinY(0.0);
        graph.getViewport().setMaxY(350);
        graph.getViewport().setYAxisBoundsManual(true);
        graph.getViewport().setXAxisBoundsManual(true);
    }

    public void onChangeActivityClick(View view){
        Intent intent=new Intent(this,MainActivity.class);
        startActivity(intent);
    }

    public void onClearClick(View view){
        mCEP.setChecked(false);
        mPPROC.setChecked(false);
        mSIFT.setChecked(false);
        mAUTOC.setChecked(false);
    }

    public void checkRadioButton(View view) {
        int radioId = radioGroup.getCheckedRadioButtonId();
        radioButton = (RadioButton) findViewById(radioId);
        Toast.makeText(this, "Selected Radio Button: " + radioButton.getText(),
                Toast.LENGTH_SHORT).show();
    }

    public void onAnalyzeClick(View view) throws IOException, WavFileException {
        if (!mCEP.isChecked() && !mPPROC.isChecked() && !mSIFT.isChecked() && !mAUTOC.isChecked()){
            Toast.makeText(this, "Please select at least one algorithm!",
                    Toast.LENGTH_SHORT).show();
            return;
        }
        int radioId = radioGroup.getCheckedRadioButtonId();
        radioButton = (RadioButton) findViewById(radioId);
        //Play audio associated with radioButton
        if(radioButton.getText().equals("Child_247")){
            Toast.makeText(this, "Analyzing Child audio",
                    Toast.LENGTH_SHORT).show();
            audioChild.start();
            InputStream inputStream=getResources().openRawResource(R.raw.c_247);
            WavFile wavFile=WavFile.openWavFile(inputStream);
            process_frames(wavFile);
        }
        else if(radioButton.getText().equals("Female_185")){
            Toast.makeText(this, "Analyzing Female audio",
                    Toast.LENGTH_SHORT).show();
            audioFemale.start();
            InputStream inputStream=getResources().openRawResource(R.raw.f_185);
            WavFile wavFile=WavFile.openWavFile(inputStream);
            process_frames(wavFile);
        }
        else if(radioButton.getText().equals("Male_82")){
            Toast.makeText(this, "Analyzing Male audio",
                    Toast.LENGTH_SHORT).show();
            audioMale.start();
            InputStream inputStream=getResources().openRawResource(R.raw.m_82);
            WavFile wavFile=WavFile.openWavFile(inputStream);
            process_frames(wavFile);
        }
    }

    public void process_frames(WavFile wavFile) throws IOException, WavFileException {
        int num_frames = (int)(wavFile.getNumFrames()/FRAME_SIZE);
        float[] data= new float[(int)wavFile.getNumFrames()];
        wavFile.readFrames(data,(int)wavFile.getNumFrames());
        float[] curr_frame = new float[FRAME_SIZE];
        //float[] pitch_values=new float[num_frames];
        graph.removeAllSeries();
        PointsGraphSeries<DataPoint> series_CEP = new PointsGraphSeries<>();
        PointsGraphSeries<DataPoint> series_PPROC = new PointsGraphSeries<>();
        PointsGraphSeries<DataPoint> series_SIFT = new PointsGraphSeries<>();
        PointsGraphSeries<DataPoint> series_AUTOC = new PointsGraphSeries<>();

        series_CEP.setTitle("CEP");
        series_PPROC.setTitle("PPROC");
        series_SIFT.setTitle("SIFT");
        series_AUTOC.setTitle("AUTOC");

        for(int i=0; i<num_frames; i++){
            curr_frame = Arrays.copyOfRange(data, i*FRAME_SIZE, (i+1)*FRAME_SIZE);
            if(mAUTOC.isChecked()){
                series_AUTOC.appendData(new DataPoint(i*0.04,getUpdate(curr_frame,0)), true,num_frames);
            }
            if(mCEP.isChecked()){
                series_CEP.appendData(new DataPoint(i*0.04,getUpdate(curr_frame,1)), true,num_frames);
            }
            if(mPPROC.isChecked()){
                series_PPROC.appendData(new DataPoint(i*0.04,getUpdate(curr_frame,2)), true,num_frames);
            }
            if(mSIFT.isChecked()){
                series_SIFT.appendData(new DataPoint(i*0.04,getUpdate(curr_frame,3)), true,num_frames);
            }
        }
        if(mAUTOC.isChecked()) {
            graph.addSeries(series_AUTOC);
            series_AUTOC.setShape(PointsGraphSeries.Shape.POINT);
            series_AUTOC.setSize(radius);
            series_AUTOC.setColor(Color.RED);

        }
        if(mCEP.isChecked()){
            graph.addSeries(series_CEP);
            series_CEP.setShape(PointsGraphSeries.Shape.POINT);
            series_CEP.setSize(radius);
            series_CEP.setColor(Color.GREEN);
        }
        if(mPPROC.isChecked()){
            graph.addSeries(series_PPROC);
            series_PPROC.setShape(PointsGraphSeries.Shape.POINT);
            series_PPROC.setSize(radius);
            series_PPROC.setColor(Color.BLUE);
        }
        if(mSIFT.isChecked()){
            graph.addSeries(series_SIFT);
            series_SIFT.setShape(PointsGraphSeries.Shape.POINT);
            series_SIFT.setSize(radius);
            series_SIFT.setColor(Color.YELLOW);
        }
        graph.getLegendRenderer().setVisible(true);

    }

    public static native float getUpdate(float [] curr_frame, int algo);

}




