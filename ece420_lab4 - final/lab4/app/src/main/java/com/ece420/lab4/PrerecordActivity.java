package com.ece420.lab4;
import com.ece420.lab4.*;
import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.GridLabelRenderer;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.LineGraphSeries;
import com.jjoe64.graphview.series.PointsGraphSeries;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaPlayer;
import android.media.MediaRecorder;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.renderscript.ScriptGroup;
import android.support.annotation.Nullable;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import org.w3c.dom.Text;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;
import java.util.Timer;
import java.util.TimerTask;

public class PrerecordActivity extends Activity {
    private static final String TAG ="Prerecord" ;
    CheckBox mCEP,mPPROC,mSIFT,mAUTOC;
    //RadioButton mChild,mFemale,mMale;
    RadioButton radioButton,mChild,mFemale,mMale;
    RadioGroup radioGroup;
    Button clear,analyze,record,recordStop,analyzeRecord;
    MediaPlayer audioChild,audioFemale,audioMale;
    GraphView graph;
    TextView cep_time,pproc_time,sift_time,autoc_time;
    TextView cep_avg,pproc_avg,sift_avg,autoc_avg;
    TextView cep_std,pproc_std,sift_std,autoc_std;
    TextView cep_vuv,pproc_vuv,sift_vuv,autoc_vuv;
    TextView cep_ferror,pproc_ferror,sift_ferror,autoc_ferror;
    TextView cep_std_error,pproc_std_error,sift_std_error,autoc_std_error;

    // Static Values
    private static final int AUDIO_ECHO_REQUEST = 0;
    private static final int FRAME_SIZE = 2048;
    private static final int radius=5;

    private float computation_time_cep=0,computation_time_pproc=0,computation_time_sift=0, computation_time_autoc=0;
    private float ground_truth=247;
    //Audio record
    AudioRecord ar = null;
    int buffsize = 0;

    int blockSize = 256;
    boolean isRecording = false;
    private Thread recordingThread = null;
    MediaPlayer player = new MediaPlayer();
    String filePath = Environment.getExternalStorageDirectory().getAbsolutePath()+"/record_temp.wav";
    WavRecorder wavRecorder=new WavRecorder(filePath);
    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] STORAGE_PERMISSIONS = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    };

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_prerecord);

        mCEP=(CheckBox)findViewById(R.id.CEP);
        mPPROC=(CheckBox)findViewById(R.id.PPROC);
        mSIFT=(CheckBox)findViewById(R.id.SIFT);
        mAUTOC=(CheckBox)findViewById(R.id.AUTOC);
        clear=(Button)findViewById(R.id.clear);
        analyze=(Button)findViewById(R.id.capture_control_button_prerecord);
        mChild=(RadioButton)findViewById(R.id.record_c);
        mFemale=(RadioButton)findViewById(R.id.record_f);
        mMale=(RadioButton)findViewById(R.id.record_m);
        record=(Button)findViewById(R.id.record_button);
        analyzeRecord=(Button)findViewById(R.id.capture_control_button_recording);
        recordStop=(Button)findViewById(R.id.record_stop);
        recordStop.setEnabled(false);

        radioGroup=(RadioGroup)findViewById(R.id.radio_group);
        audioChild=MediaPlayer.create(PrerecordActivity.this,R.raw.c_247);
        audioFemale=MediaPlayer.create(PrerecordActivity.this,R.raw.f_185);
        audioMale=MediaPlayer.create(PrerecordActivity.this,R.raw.m_82);
        graph = (GraphView) findViewById(R.id.graph);
        GridLabelRenderer gridLabel = graph.getGridLabelRenderer();
        gridLabel.setHorizontalAxisTitle("Time (s)");
        gridLabel.setVerticalAxisTitle("Pitch (Hz)");
        graph.getViewport().setMinX(0);
        graph.getViewport().setMaxX(4);
        graph.getViewport().setMinY(0.0);
        graph.getViewport().setMaxY(350);
        graph.getViewport().setYAxisBoundsManual(true);
        graph.getViewport().setXAxisBoundsManual(true);
        graph.getViewport().setScalable(true);  // activate horizontal zooming and scrolling
        graph.getViewport().setScrollable(true);  // activate horizontal scrolling
        graph.getViewport().setScalableY(true);  // activate horizontal and vertical zooming and scrolling
        graph.getViewport().setScrollableY(true);  // activate vertical scrolling

        cep_time=(TextView)findViewById(R.id.cep_time);
        pproc_time=(TextView)findViewById(R.id.pproc_time);
        sift_time=(TextView)findViewById(R.id.sift_time);
        autoc_time=(TextView)findViewById(R.id.autoc_time);

        cep_avg=(TextView)findViewById(R.id.cep_avg);
        pproc_avg=(TextView)findViewById(R.id.pproc_avg);
        sift_avg=(TextView)findViewById(R.id.sift_avg);
        autoc_avg=(TextView)findViewById(R.id.autoc_avg);

        cep_std=(TextView)findViewById(R.id.cep_std);
        pproc_std=(TextView)findViewById(R.id.pproc_std);
        sift_std=(TextView)findViewById(R.id.sift_std);
        autoc_std=(TextView)findViewById(R.id.autoc_std);

        cep_vuv=(TextView)findViewById(R.id.cep_vuv);
        pproc_vuv=(TextView)findViewById(R.id.pproc_vuv);
        sift_vuv=(TextView)findViewById(R.id.sift_vuv);
        autoc_vuv=(TextView)findViewById(R.id.autoc_vuv);

        cep_ferror=(TextView)findViewById(R.id.cep_ferror);
        pproc_ferror=(TextView)findViewById(R.id.pproc_ferror);
        sift_ferror=(TextView)findViewById(R.id.sift_ferror);
        autoc_ferror=(TextView)findViewById(R.id.autoc_ferror);

        cep_std_error=(TextView)findViewById(R.id.cep_std_error);
        pproc_std_error=(TextView)findViewById(R.id.pproc_std_error);
        sift_std_error=(TextView)findViewById(R.id.sift_std_error);
        autoc_std_error=(TextView)findViewById(R.id.autoc_std_error);
        verifyStoragePermissions();
    }
    public void verifyStoragePermissions() {
        // Check if we have write permission
        Log.d(TAG, "verifyPermissions: Checking Permissions.");

        int permissionExternalMemory = ActivityCompat.checkSelfPermission(PrerecordActivity.this, Manifest.permission.WRITE_EXTERNAL_STORAGE);


        if (permissionExternalMemory != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(
                    PrerecordActivity.this,
                    STORAGE_PERMISSIONS,
                    1
            );
        }
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
    public void onRecordClick(View view) throws IOException, WavFileException {
        recordStop.setEnabled(true);
        setAnalyzeButtonsStatus(false);
        if(!isRecording){
            record.setEnabled(false);
            wavRecorder.startRecording();
            isRecording=true;
            Toast.makeText(this, "Start recording",
                    Toast.LENGTH_SHORT).show();
        }
        setAnalyzeButtonsStatus(true);
    }

    public void onStopClick(View view) throws IOException, WavFileException {
        record.setEnabled(true);
        setAnalyzeButtonsStatus(true);
        if(!isRecording){
            Toast.makeText(this, "Recording has not started.",
                    Toast.LENGTH_SHORT).show();
        }
        else {
            wavRecorder.stopRecording();
            isRecording = false;
            Toast.makeText(this, "Stop recording",
                    Toast.LENGTH_SHORT).show();
            recordStop.setEnabled(false);
        }
    }

    public void onAnalyzeRecordingClick(View view) throws IOException, WavFileException {
        record.setEnabled(false);
        recordStop.setEnabled(false);
        setAnalyzeButtonsStatus(false);
        if(!isRecording){
            player.stop();
            player.reset();
            try {
                File file = new File(filePath);
                if(!file.exists()){
                    Toast.makeText(this, "You don't have any recording on file!",
                            Toast.LENGTH_SHORT).show();
                    analyzeRecord.setEnabled(true);
                    analyze.setEnabled(true);
                    return;
                }
                player.setDataSource(filePath);
                player.prepare();
                player.start();
            } catch (Exception e) {
                e.printStackTrace();
            }
            ground_truth=0;
            FileInputStream inputStream=new FileInputStream(filePath);
            WavFile wavFile=WavFile.openWavFile(inputStream);
            process_frames(wavFile);
        }
        setRecordButtonsStatus(true);
        setAnalyzeButtonsStatus(true);
    }
//    public void onRecordClick(View view)
//    {
//        // when click to START
//        if(!isRecording){
//            buffsize = AudioRecord.getMinBufferSize(44100, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT);
//            ar = new AudioRecord(MediaRecorder.AudioSource.MIC, 44100, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT, buffsize);
//
//            ar.startRecording();
//
//            isRecording = true;
//            recordingThread = new Thread(new Runnable() {
//                public void run() {
//                    try {
//                        writeAudioDataToFile();
//                    } catch (IOException e) {
//                        e.printStackTrace();
//                    }
//                }
//            }, "AudioRecorder Thread");
//            recordingThread.start();
//        }
//        else{
//            ar.stop();
//            isRecording = false;
//            try {
//                player.setDataSource(filePath);
//                player.prepare();
//                player.start();
//            } catch (Exception e) {
//                e.printStackTrace();
//            }
//
//        }
//
//    }

//    private void writeAudioDataToFile() throws IOException {
//        // Write the output audio in byte
//
//        //String filePath = "/sdcard/voice8K16bitmono.wav";
//        File file=new File(filePath);
//        if(!file.exists()){
//            file.createNewFile();
//        }
//
//        short sData[] = new short[buffsize/2];
//
//        FileOutputStream os = null;
//        try {
//            os = new FileOutputStream(filePath);
//        } catch (FileNotFoundException e) {
//            e.printStackTrace();
//        }
//
//        while (isRecording) {
//            // gets the voice output from microphone to byte format
//            ar.read(sData, 0, buffsize/2);
//            Log.d("eray","Short writing to file" + sData.toString());
//            try {
//                // // writes the data to file from buffer
//                // // stores the voice buffer
//                byte bData[] = short2byte(sData);
//                os.write(bData, 0, buffsize);
//            } catch (IOException e) {
//                e.printStackTrace();
//            }
//        }
//        try {
//            os.close();
//        } catch (IOException e) {
//            e.printStackTrace();
//        }
//    }
//
//    private byte[] short2byte(short[] sData) {
//        int shortArrsize = sData.length;
//        byte[] bytes = new byte[shortArrsize * 2];
//        for (int i = 0; i < shortArrsize; i++) {
//            bytes[i * 2] = (byte) (sData[i] & 0x00FF);
//            bytes[(i * 2) + 1] = (byte) (sData[i] >> 8);
//            sData[i] = 0;
//        }
//        return bytes;
//
//    }

    public void checkRadioButton(View view) {
        int radioId = radioGroup.getCheckedRadioButtonId();
        radioButton = (RadioButton) findViewById(radioId);
        Toast.makeText(this, "Selected Radio Button: " + radioButton.getText(),
                Toast.LENGTH_SHORT).show();
    }

    public void setAnalyzeButtonsStatus(boolean value){
        analyze.setEnabled(value);
        analyzeRecord.setEnabled(value);
    }

    public void setRecordButtonsStatus(boolean value){
        record.setEnabled(value);
        recordStop.setEnabled(!value);
    }
    public void onAnalyzeClick(View view) throws IOException, WavFileException {
        clearMetric();
        record.setEnabled(false);
        recordStop.setEnabled(false);
        setAnalyzeButtonsStatus(false);
        if (!mCEP.isChecked() && !mPPROC.isChecked() && !mSIFT.isChecked() && !mAUTOC.isChecked()){
            Toast.makeText(this, "Please select at least one algorithm!",
                    Toast.LENGTH_SHORT).show();
            setRecordButtonsStatus(true);
            setAnalyzeButtonsStatus(true);
            return;
        }
        if (!mChild.isChecked() && !mFemale.isChecked() && !mMale.isChecked()){
            Toast.makeText(this, "Please select at least one sound file!",
                    Toast.LENGTH_SHORT).show();
            setRecordButtonsStatus(true);
            setAnalyzeButtonsStatus(true);
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
            ground_truth=247;
            WavFile wavFile=WavFile.openWavFile(inputStream);
            process_frames(wavFile);
        }
        else if(radioButton.getText().equals("Female_185")){
            Toast.makeText(this, "Analyzing Female audio",
                    Toast.LENGTH_SHORT).show();
            audioFemale.start();
            ground_truth=185;
            InputStream inputStream=getResources().openRawResource(R.raw.f_185);
            WavFile wavFile=WavFile.openWavFile(inputStream);
            process_frames(wavFile);
        }
        else if(radioButton.getText().equals("Male_82")){
            Toast.makeText(this, "Analyzing Male audio",
                    Toast.LENGTH_SHORT).show();
            audioMale.start();
            ground_truth=82;
            InputStream inputStream=getResources().openRawResource(R.raw.m_82);
            WavFile wavFile=WavFile.openWavFile(inputStream);
            process_frames(wavFile);
        }
        setRecordButtonsStatus(true);
        setAnalyzeButtonsStatus(true);
    }

    public void process_frames(WavFile wavFile) throws IOException, WavFileException {
        int num_frames = (int)(wavFile.getNumFrames()/FRAME_SIZE);
        float[] data= new float[(int)wavFile.getNumFrames()];
        wavFile.readFrames(data,(int)wavFile.getNumFrames());
        float[] curr_frame = new float[FRAME_SIZE];
        float[] pitch_values_autoc=new float[num_frames];
        float[] pitch_values_cep=new float[num_frames];
        float[] pitch_values_pproc=new float[num_frames];
        float[] pitch_values_sift=new float[num_frames];
        long time_autoc=0;
        long time_cep=0;
        long time_pproc=0;
        long time_sift=0;
        long time_stamp;

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
                time_stamp = System.currentTimeMillis();
                pitch_values_autoc[i]=Math.max(0,getUpdate(curr_frame,0));
                time_autoc+=System.currentTimeMillis()-time_stamp;
                series_AUTOC.appendData(new DataPoint(i*0.04,pitch_values_autoc[i]), true,num_frames);
            }
            if(mCEP.isChecked()){
                time_stamp = System.currentTimeMillis();
                pitch_values_cep[i]=Math.max(0,getUpdate(curr_frame,1));
                time_cep+=System.currentTimeMillis()-time_stamp;
                series_CEP.appendData(new DataPoint(i*0.04,pitch_values_cep[i]), true,num_frames);
            }
            if(mPPROC.isChecked()){
                time_stamp = System.currentTimeMillis();
                pitch_values_pproc[i]=Math.max(0,getUpdate(curr_frame,2));
                time_pproc+=System.currentTimeMillis()-time_stamp;
                series_PPROC.appendData(new DataPoint(i*0.04,pitch_values_pproc[i]), true,num_frames);
            }
            if(mSIFT.isChecked()){
                time_stamp = System.currentTimeMillis();
                pitch_values_sift[i]=Math.max(0,getUpdate(curr_frame,3));
                time_sift+=System.currentTimeMillis()-time_stamp;
                series_SIFT.appendData(new DataPoint(i*0.04,pitch_values_sift[i]), true,num_frames);
            }
        }
        if(mAUTOC.isChecked()) {
            graph.addSeries(series_AUTOC);
            series_AUTOC.setShape(PointsGraphSeries.Shape.POINT);
            series_AUTOC.setSize(radius);
            series_AUTOC.setColor(Color.RED);
            calculateMetric(pitch_values_autoc,0);
            autoc_time.setText(String.format("%.2f", (float)time_autoc));
        }
        if(mCEP.isChecked()){
            graph.addSeries(series_CEP);
            series_CEP.setShape(PointsGraphSeries.Shape.POINT);
            series_CEP.setSize(radius);
            series_CEP.setColor(Color.GREEN);
            calculateMetric(pitch_values_cep,1);
            cep_time.setText(String.format("%.2f", (float)time_cep));
        }
        if(mPPROC.isChecked()){
            graph.addSeries(series_PPROC);
            series_PPROC.setShape(PointsGraphSeries.Shape.POINT);
            series_PPROC.setSize(radius);
            series_PPROC.setColor(Color.BLUE);
            calculateMetric(pitch_values_pproc,2);
            pproc_time.setText(String.format("%.2f", (float)time_pproc));
        }
        if(mSIFT.isChecked()){
            graph.addSeries(series_SIFT);
            series_SIFT.setShape(PointsGraphSeries.Shape.POINT);
            series_SIFT.setSize(radius);
            series_SIFT.setColor(Color.YELLOW);
            calculateMetric(pitch_values_sift,3);
            sift_time.setText(String.format("%.2f", (float)time_sift));

        }
        graph.getLegendRenderer().setVisible(true);

    }

    public void calculateMetric(float[] pitch, int algo){
        int voiced_num=0,ferror_num=0;
        float total=0,error=0;
        float avg=0,std=0,ferror=0,std_error=0;
        for(int i=0;i<pitch.length;i++) {
            if (pitch[i] > 0) {
                voiced_num++;
                total += pitch[i];
                if(ground_truth>0){
                    error=Math.abs(pitch[i]-ground_truth);
                    if(error<0.001*44100){
                        ferror+= Math.abs(pitch[i]-ground_truth);
                        std_error+=Math.abs(pitch[i]-ground_truth)*Math.abs(pitch[i]-ground_truth);
                        ferror_num++;
                    }
                }
            }
        }
        float vuv=(float)voiced_num/pitch.length;
        if(voiced_num>0) {
            avg=(float)total/voiced_num;
        }
        if(ferror_num>0){
            ferror = (float)ferror/voiced_num;
            std_error=(float)Math.sqrt(std_error/voiced_num-ferror*ferror);
        }
        for(int i=0;i<pitch.length;i++){
            if(pitch[i]>0){
                std+=(pitch[i]-avg)*(pitch[i]-avg);
            }
        }
        std=(float)Math.sqrt(std/voiced_num);

        //String.format("%.2f", value)
        if(algo==0){
            autoc_avg.setText(String.format("%.2f", avg));
            autoc_std.setText(String.format("%.2f", std));
            autoc_vuv.setText(String.format("%.2f%%", vuv));
            autoc_ferror.setText(String.format("%.2f", ferror));
            autoc_std_error.setText(String.format("%.2f", std_error));
        }
        if(algo==1){
            cep_avg.setText(String.format("%.2f", avg));
            cep_std.setText(String.format("%.2f", std));
            cep_vuv.setText(String.format("%.2f%%", vuv));
            cep_ferror.setText(String.format("%.2f", ferror));
            cep_std_error.setText(String.format("%.2f", std_error));
        }
        if(algo==2){
            pproc_avg.setText(String.format("%.2f", avg));
            pproc_std.setText(String.format("%.2f", std));
            pproc_vuv.setText(String.format("%.2f%%", vuv));
            pproc_ferror.setText(String.format("%.2f", ferror));
            pproc_std_error.setText(String.format("%.2f", std_error));
        }
        if(algo==3){
            sift_avg.setText(String.format("%.2f", avg));
            sift_std.setText(String.format("%.2f", std));
            sift_vuv.setText(String.format("%.2f%%", vuv));
            sift_ferror.setText(String.format("%.2f", ferror));
            sift_std_error.setText(String.format("%.2f", std_error));
        }
        if(ground_truth==0){
            cep_ferror.setText("N/a");
            pproc_ferror.setText("N/a");
            sift_ferror.setText("N/a");
            autoc_ferror.setText("N/a");
            cep_std_error.setText("N/a");
            pproc_std_error.setText("N/a");
            sift_std_error.setText("N/a");
            autoc_std_error.setText("N/a");
        }
    }

    public void clearMetric(){
        cep_time.setText("N/a");
        pproc_time.setText("N/a");
        sift_time.setText("N/a");
        autoc_time.setText("N/a");
        cep_avg.setText("N/a");
        pproc_avg.setText("N/a");
        sift_avg.setText("N/a");
        autoc_avg.setText("N/a");
        cep_std.setText("N/a");
        pproc_std.setText("N/a");
        sift_std.setText("N/a");
        autoc_std.setText("N/a");
        cep_vuv.setText("N/a");
        pproc_vuv.setText("N/a");
        sift_vuv.setText("N/a");
        autoc_vuv.setText("N/a");
        cep_ferror.setText("N/a");
        pproc_ferror.setText("N/a");
        sift_ferror.setText("N/a");
        autoc_ferror.setText("N/a");
        cep_std_error.setText("N/a");
        pproc_std_error.setText("N/a");
        sift_std_error.setText("N/a");
        autoc_std_error.setText("N/a");
    }
    @Override
    protected void onPause() {
        //stop mediaplayer:
        if (player.isPlaying()) {
            player.stop();
        }
        super.onPause();
    }
    public static native float getUpdate(float [] curr_frame, int algo);

}




