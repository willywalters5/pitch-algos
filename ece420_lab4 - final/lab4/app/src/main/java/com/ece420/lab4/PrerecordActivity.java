package com.ece420.lab4;

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

public class PrerecordActivity extends Activity {
    CheckBox mCEP,mPPROC,mSIFT,mAUTOC;
    //RadioButton mChild,mFemale,mMale;
    RadioButton radioButton;
    RadioGroup radioGroup;
    Button clear;
    MediaPlayer audioChild,audioFemale,audioMale;


    ImageView stftView;
    Bitmap bitmap;
    Canvas canvas;
    Paint paint;
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

        stftView = (ImageView) this.findViewById(R.id.stftView);
        bitmap =  Bitmap.createBitmap((FRAME_SIZE), BITMAP_HEIGHT, Bitmap.Config.ARGB_8888);
        canvas = new Canvas(bitmap);
        canvas.drawColor(Color.BLACK);
        paint = new Paint();
        paint.setColor(Color.GREEN);
        paint.setStyle(Paint.Style.FILL);
        stftView.setImageBitmap(bitmap);
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

    public void onAnalyzeClick(View view){
        if (!mCEP.isChecked() && !mPPROC.isChecked() && !mSIFT.isChecked() && !mAUTOC.isChecked()){
            Toast.makeText(this, "Please select at least one algorithm!",
                    Toast.LENGTH_SHORT).show();
            return;
        }
        int radioId = radioGroup.getCheckedRadioButtonId();
        radioButton = (RadioButton) findViewById(radioId);
        //Play audio associated with radioButton
        if(radioButton.getText().equals("Child_247")){
            Toast.makeText(this, "Playing Child audio",
                    Toast.LENGTH_SHORT).show();
            audioChild.start();
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
}




