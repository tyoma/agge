package impression.sandbox;

import android.app.Activity;
import android.os.Bundle;
import android.content.Context;
import android.view.View;
import android.graphics.*;
import android.widget.TextView;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.util.AttributeSet;
import java.lang.Math;

public class SandboxView extends View {
	private Bitmap mBitmap;
	private long mDuration;
	private long mTimes;
	private boolean mUseAGG;
	TextView mStatsView;
	Path mSpiral;

	public SandboxView(Context context, AttributeSet attributes) {
		super(context, attributes);

		mUseAGG = true;
		mDuration = 0;
		mTimes = 0;
		aggObject = 0;
	}

	public void setStatsView(TextView textview)
	{
		mStatsView = textview;
	}

	public void setUseAGG(boolean useAGG)
	{
		mUseAGG = useAGG;
		mDuration = 0;
		mTimes = 0;
	}

	private void updateSpiral(float x, float y, float r1, float r2, float step, float angle)
	{
		float r = r1, prevx = x, prevy = y;
		float dr = step / 45.0f;
		float da = 3.1415926f / 180.0f;

		mSpiral = new Path();
		mSpiral.moveTo(x + (float)Math.cos(angle) * r, y + (float)Math.sin(angle) * r);

		while (r <= r2)
		{
			mSpiral.lineTo(x + (float)Math.cos(angle) * r, y + (float)Math.sin(angle) * r);
			angle += da;
			r += dr;
		}
	}

	@Override protected void onSizeChanged(int w, int h, int oldw, int oldh)
	{
		super.onSizeChanged(w, h, oldw, oldh);
		mBitmap = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
		updateSpiral(w / 2, h / 2, 5, (w < h ? w : h) / 2 - 10, 1, 0);
		updateSize(w, h);
	}

	@Override protected void onAttachedToWindow()
	{
		constructAGG();
	}

	@Override protected void onDetachedFromWindow()
	{
		destroyAGG();
	}

	@Override protected void onDraw(Canvas canvas)
	{
		Paint paint = new Paint();

		canvas.drawColor(Color.WHITE);
		mBitmap.eraseColor(Color.WHITE);
		paint.setStyle(Paint.Style.STROKE);
		paint.setStrokeJoin(Paint.Join.ROUND);
		paint.setStrokeCap(Paint.Cap.ROUND);
		paint.setARGB(255, 0, 154, 255);
		paint.setStrokeWidth(3);
		paint.setAntiAlias(true);

		long started = System.nanoTime();

		if (mUseAGG)
		{
			render(mBitmap);
			canvas.drawBitmap(mBitmap, 0, 0, null);
		}
		else
		{
			canvas.drawPath(mSpiral, paint);
		}

		long ended = System.nanoTime();

		mDuration += ended - started;
		if (32 == mTimes++ && mStatsView != null)
		{
			mDuration /= 32;
			mStatsView.setText(String.format("Rendition time: %fms", 1e-6 * (double)mDuration));
			mDuration = 0;
			mTimes = 0;
		}

		// force a redraw, with a different time-based pattern.
		invalidate();
	}

	private long aggObject;

	private native void constructAGG();
	private native void render(Bitmap bitmap);
	private native void updateSize(int width, int height);
	private native void destroyAGG();
}
