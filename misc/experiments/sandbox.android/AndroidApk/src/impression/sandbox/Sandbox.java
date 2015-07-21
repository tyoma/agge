/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package impression.sandbox;

import android.app.Activity;
import android.os.Bundle;
import android.content.Context;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.TextView;
import android.graphics.Bitmap;
import android.graphics.Canvas;

public class Sandbox extends Activity
{
	private SandboxView mSandboxView;
	private CheckBox mUseAGGCheckBox;

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		
		mSandboxView = (SandboxView)findViewById(R.id.sandboxView);
		mUseAGGCheckBox = (CheckBox)findViewById(R.id.useAGG);

		mSandboxView.setStatsView((TextView)findViewById(R.id.stats));
		mSandboxView.setUseAGG(true);
		mUseAGGCheckBox.setChecked(true);
	}

	public void onUseAGGChanged(View checkbox)
	{
		mSandboxView.setUseAGG(((CheckBox)checkbox).isChecked());
	}

	static
	{
		System.loadLibrary("sandbox.android");
	}
}
