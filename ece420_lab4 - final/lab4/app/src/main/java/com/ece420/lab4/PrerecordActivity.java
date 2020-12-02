package com.ece420.lab4;
import com.ece420.lab4.*;
import com.jjoe64.graphview.GraphView;
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
import android.os.Bundle;
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
    private static final int BITMAP_HEIGHT = 500;

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
//            Toast.makeText(this, "Playing Child audio",
//                    Toast.LENGTH_SHORT).show();
            //audioChild.start();
            InputStream inputStream=getResources().openRawResource(R.raw.c_247);
            WavFile wavFile=WavFile.openWavFile(inputStream);
            process_frames(wavFile);
        }
        else if(radioButton.getText().equals("Female_185")){
            Toast.makeText(this, "Playing Female audio",
                    Toast.LENGTH_SHORT).show();
            audioFemale.start();
        }
        else if(radioButton.getText().equals("Male_82")){
            Toast.makeText(this, "Playing Male audio",
                    Toast.LENGTH_SHORT).show();
            audioMale.start();
        }
    }

    public void process_frames(WavFile wavFile) throws IOException, WavFileException {
        int num_frames = (int)(wavFile.getNumFrames()/FRAME_SIZE);
        float[] data= new float[(int)wavFile.getNumFrames()];
        wavFile.readFrames(data,(int)wavFile.getNumFrames());
        float[] curr_frame = new float[FRAME_SIZE];
        float[] pitch_values=new float[num_frames];
        PointsGraphSeries<DataPoint> series = new PointsGraphSeries<>();
        for(int i=0; i<num_frames; i++){
            curr_frame = Arrays.copyOfRange(data, i*FRAME_SIZE, (i+1)*FRAME_SIZE);
            pitch_values[i]=getCEPUpdate(curr_frame);
            series.appendData(new DataPoint(i*0.04,pitch_values[i]), true,num_frames);
        }
        graph.addSeries(series);
        series.setShape(PointsGraphSeries.Shape.POINT);
    }

    public static native float getCEPUpdate(float [] curr_frame);

}




