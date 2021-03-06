package com.teevid.sample;

import android.content.Intent;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.Toast;
import android.widget.ToggleButton;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.StringRes;
import androidx.appcompat.app.AlertDialog;
import androidx.core.util.Consumer;
import androidx.fragment.app.Fragment;

import com.teevid.sdk.TeeVidClient;
import com.teevid.sdk.TeeVidEventListener;
import com.teevid.sdk.api.SdkErrors;
import com.teevid.sdk.log.LogLevel;
import com.teevid.sdk.view.BaseMeetingView;

public class CallFragment extends Fragment {

    private static final String TAG = "CallFragment";

    private TeeVidClient client;

    private ToggleButton btnMicrophone;
    private ToggleButton btnCamera;

    public CallFragment() {

    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_call, container, false);
        return view;
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        BaseMeetingView viewTeeVid = view.findViewById(R.id.view_meeting);
        btnCamera = view.findViewById(R.id.btn_camera);
        ToggleButton btnLocalVideo = view.findViewById(R.id.btn_local_video);
        btnMicrophone = view.findViewById(R.id.btn_microphone);
        ToggleButton btnScreenShare = view.findViewById(R.id.btn_screen_share);
        ToggleButton btnCameraSwitch = view.findViewById(R.id.btn_camera_switch);

        UserPreferences preferences = SampleApplication.getInstance().getUserPreferences();
        String server = preferences.getServer();
        String roomId = preferences.getRoomId();
        String username = preferences.getUsername();
        String invitationLink = preferences.getInvitationLink();
        int defaultCamera = preferences.getCamera();

        client = new TeeVidClient.Builder(getContext(), "token") // TODO Token
                .addListener(getEventListener())
                .setLogLevel(LogLevel.DEBUG)
                .build();
        client.setView(viewTeeVid);
        client.setDefaultCamera(defaultCamera);

        if (TextUtils.isEmpty(invitationLink)) {
            client.connect(roomId, server, username);
        } else {
            client.connectWithInvitation(username, invitationLink);
        }

        btnCamera.setOnClickListener(v -> onCameraButtonClicked(btnCamera));
        btnLocalVideo.setOnClickListener(v -> onLocalVideoButtonClicked(btnLocalVideo));
        btnScreenShare.setOnClickListener(v -> onScreenShareButtonClicked(btnScreenShare));
        btnMicrophone.setOnClickListener(v -> onMicrophoneButtonClicked(btnMicrophone));
        btnCameraSwitch.setOnClickListener(v -> onSwitchCameraButtonClicked());
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        client.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        client.disconnect();
    }

    private void onCameraButtonClicked(ToggleButton button) {
        boolean enabled = button.isChecked(); // Button's state changes before onClick is fired
        if (enabled) {
            client.resumeVideo();
        } else {
            client.stopVideo();
        }
    }

    private void onLocalVideoButtonClicked(ToggleButton button) {
        boolean enabled = button.isChecked();
        if (enabled) {
            client.showLocalVideo();
        } else {
            client.hideLocalVideo();
        }
    }

    private void onScreenShareButtonClicked(ToggleButton button) {
        boolean enabled = button.isChecked();
        if (enabled) {
            client.stopScreenShare();
        } else {
            client.startScreenShare(this);
        }
    }

    private void onMicrophoneButtonClicked(ToggleButton button) {
        boolean enabled = button.isChecked();
        if (enabled) {
            client.unmute();
        } else {
            client.mute();
        }
    }

    private void onSwitchCameraButtonClicked() {
        client.switchCamera();
    }

    private void showEnterPinDialog(Consumer<String> pinConsumer) {
        View view = LayoutInflater.from(getContext()).inflate(R.layout.dialog_edit_text, null);
        EditText editText = view.findViewById(R.id.et_input);
        editText.setHint(R.string.enter_pin);

        AlertDialog dialog = new AlertDialog.Builder(getContext())
                .setTitle(R.string.enter_pin_title)
                .setPositiveButton(R.string.dialog_btn_connect, (dialog1, which) -> {
                    String pin = editText.getText().toString();
                    pinConsumer.accept(pin);
                })
                .setNegativeButton(R.string.dialog_btn_cancel, null)
                .setCancelable(false)
                .setView(view)
                .create();

        dialog.show();
    }

    private void showUnmuteAudioRequestDialog(Consumer<Boolean> resultConsumer) {
        AlertDialog alertDialog = new AlertDialog.Builder(getContext())
                .setTitle(R.string.dialog_unmute_audio_title)
                .setPositiveButton(R.string.dialog_unmute_positive, (dialog, which) -> {
                    resultConsumer.accept(true);
                    btnMicrophone.setVisibility(View.VISIBLE);
                    if (btnMicrophone.isChecked()) {
                        btnMicrophone.toggle();
                    }
                    showToast(R.string.dialog_unmute_microphone_unmuted);
                })
                .setNegativeButton(R.string.dialog_unmute_negative, (dialog, which) ->
                        resultConsumer.accept(false))
                .create();

        alertDialog.show();
    }

    private void showUnmuteVideoRequestDialog(Consumer<Boolean> resultConsumer) {
        AlertDialog alertDialog = new AlertDialog.Builder(getContext())
                .setTitle(R.string.dialog_unmute_video_title)
                .setPositiveButton(R.string.dialog_unmute_positive, (dialog, which) -> {
                    resultConsumer.accept(true);
                    btnCamera.setVisibility(View.VISIBLE);
                    if (!btnCamera.isChecked()) {
                        btnCamera.toggle();
                    }
                    showToast(R.string.dialog_unmute_camera_unmuted);
                })
                .setNegativeButton(R.string.dialog_unmute_negative, (dialog, which) ->
                        resultConsumer.accept(false))
                .create();

        alertDialog.show();
    }

    private void showToast(@StringRes int resId) {
        Toast.makeText(getContext(), resId, Toast.LENGTH_SHORT).show();
    }

    private TeeVidEventListener getEventListener() {
        return new TeeVidEventListener() {

            @Override
            public void onConnect() {
                Log.d(TAG, "onConnect");
            }

            @Override
            public void onDisconnect() {
                Log.d(TAG, "onDisconnect");
            }

            @Override
            public void onAddParticipant(String participantId) {
                Log.d(TAG, "onAddParticipant: " + participantId);
            }

            @Override
            public void onRemoveParticipant(String participantId) {
                Log.d(TAG, "onRemoveParticipant: " + participantId);
            }

            @Override
            public void onReceiveError(Throwable throwable) {
                Log.e(TAG, "onReceiveError: ", throwable);
                Toast.makeText(getContext(), throwable.getMessage(), Toast.LENGTH_LONG).show();
            }

            @Override
            public void onRequestAccessPin(String reason, Consumer<String> pinConsumer) {
                Log.d(TAG, "onRequestAccessPin: " + reason);
                if (SdkErrors.BAD_PIN.equals(reason)) {
                    Toast.makeText(getContext(), R.string.invalid_pin, Toast.LENGTH_LONG).show();
                }
                showEnterPinDialog(pinConsumer);
            }

            @Override
            public void onMuteAudioByModerator() {
                Log.d(TAG, "onMutedByModerator");
                btnMicrophone.setVisibility(View.GONE);
            }

            @Override
            public void onMuteVideoByModerator() {
                Log.d(TAG, "onMuteVideoByModerator");
                btnCamera.setVisibility(View.GONE);
            }

            @Override
            public void onReceiveUnmuteAudioRequest(Consumer<Boolean> resultConsumer) {
                Log.d(TAG, "onReceiveUnmuteAudioRequest");
                showUnmuteAudioRequestDialog(resultConsumer);
            }

            @Override
            public void onReceiveUnmuteVideoRequest(Consumer<Boolean> resultConsumer) {
                Log.d(TAG, "onReceiveUnmuteVideoRequest");
                showUnmuteVideoRequestDialog(resultConsumer);
            }

            @Override
            public void onDisconnectByModerator() {
                Log.d(TAG, "onDisconnectByModerator");
                showToast(R.string.disconnected_from_room);
            }

            @Override
            public void onChangeVoiceActivationState(boolean activated) {
                Log.d(TAG, "onChangeVoiceActivationState: " + activated);
            }
        };
    }
}
