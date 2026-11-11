

import time

import cv2
from enum import Enum, auto
from picamera2 import Picamera2
from gpiozero import DigitalInputDevice, DigitalOutputDevice
import mediapipe as mp

WIDTH, HEIGHT = 640, 480
SCREEN_CENTER = WIDTH/2
CENTER_MARGIN = 0.05
CONFIRM_MARGIN = 0.20

# input pins
PIN_DIRECTION = 3
PIN_START = 4

# output pins
PIN_LEFT = 17
PIN_RIGHT = 27
PIN_CENTERED = 22
PIN_PLAYER = 23
PIN_SKIP = 24

FRAMES_HELD_THRESH = 15
CONFIRM_TIMEOUT = 3 # seconds

# state machine for player scan
class State(Enum):
    IDLE = auto()
    SEARCH = auto() # looking for face
    CENTER = auto() # send signals to MCU to center camera on face
    CONFIRM = auto() # wait for thumbs up to authenticate
    WAIT = auto() # report person as player or skip
    
state = State.SEARCH


# returns valid face closest to screen center 
def next_target(faces, direction):
    # clockwise, so faces should be right of center
    aheads = []
    if direction == 0: 
        aheads = [i.keypoints[2].x - 0.5 if i.keypoints[2].x > 0.5 else 1 for i in faces]
        
    # otherwise counterclockwise, so faces should be left of center
    else:
        aheads = [0.5 - i.keypoints[2].x if i.keypoints[2].x < 0.5 else 1 for i in faces]
    
    # return None if there is no valid face
    if aheads:
        m = min(aheads)
        if m == 1:
            return None
        
        i = aheads.index(m)
        return faces[i]
    
    return None


def find_target(faces, target_x):
    deltas = [abs(i.keypoints[2].x - target_x) for i in faces]
    
    if not deltas:
        return None
    
    i = deltas.index(min(deltas))
    return faces[i]
    


# camera config
cam = Picamera2()
config = cam.create_preview_configuration(main={"size": (WIDTH, HEIGHT), "format": "RGB888"})
cam.configure(config)
cam.start()
time.sleep(1) # let the auto-exposure settle


# IO ports
# inp_direction = DigitalInputDevice(PIN_DIRECTION) # 0 is clockwise, 1 is counterclockwise
# inp_reset = DigitalInputDevice(PIN_RESET)

# DEBUGGING
inp_direction = False
inp_reset = False

out_right = DigitalOutputDevice(PIN_RIGHT)
out_left = DigitalOutputDevice(PIN_LEFT)
out_center = DigitalOutputDevice(PIN_CENTERED)
out_player = DigitalOutputDevice(PIN_PLAYER)
out_skip = DigitalOutputDevice(PIN_SKIP)


# mediapipe ML
BaseOptions = mp.tasks.BaseOptions

# face detection
FaceDetector = mp.tasks.vision.FaceDetector
FaceDetectorOptions = mp.tasks.vision.FaceDetectorOptions
VisionRunningMode = mp.tasks.vision.RunningMode

fd_options = FaceDetectorOptions(
    base_options=BaseOptions(model_asset_path="blaze_face_short_range.tflite"),
    running_mode=VisionRunningMode.VIDEO,
    min_detection_confidence=0.6,
)

# gesture recognition
GestureRecognizer = mp.tasks.vision.GestureRecognizer
GestureRecognizerOptions = mp.tasks.vision.GestureRecognizerOptions
VisionRunningMode = mp.tasks.vision.RunningMode
ClassifierOptions = mp.tasks.components.processors.ClassifierOptions

gr_options = GestureRecognizerOptions(
    base_options=BaseOptions(model_asset_path="gesture_recognizer.task"),
    running_mode=VisionRunningMode.VIDEO,
    num_hands=2,
    canned_gesture_classifier_options=ClassifierOptions(
        category_allowlist=["Thumb_Up"], # only detect thumbs up
        score_threshold=0.6,
    )
)


# values held throughout execution
frames_w_thumbs_up = 0
frames_w_target_centered = 0
frames_w_target = 0
frames_w_no_target = 0
person_center = 0
confirm_start = 0
closest_person_x = 0
person_type = ""

# declare start time (for frame of video)
start = time.monotonic()

with FaceDetector.create_from_options(fd_options) as fd, GestureRecognizer.create_from_options(gr_options) as gr:
    while True:
        #set outputs low at start of loop
        right = 0
        left = 0
        center = 0
        player = 0
        skip = 0
        
        direction = inp_direction
        reset = inp_reset
        
        # read camera image
        frame = cam.capture_array()
        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        
        # use mediapipe to extract an array of face and hand keypoints
        mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb)
        timestamp_ms = int((time.monotonic() - start) * 1000)
        gr_result = gr.recognize_for_video(mp_image, timestamp_ms)
        fd_result = fd.detect_for_video(mp_image, timestamp_ms)
        
        
        match state:
            # hold values low on idle, exit on "start" input
            case State.IDLE:
                frames_w_thumbs_up = 0
                frames_w_target_centered = 0
                frames_w_target = 0
                frames_w_no_target = 0
                person_center = 0
                confirm_timer = 0
                closest_person_x = 0
                person_type = ""
                
                if not reset:
                    state = State.SEARCH
                
            # if face detection sees a face, go into centering phase
            case State.SEARCH:
                closest_face = next_target(fd_result.detections, direction)
                
                if closest_face is not None:
                    frames_w_target += 1
                    
                    if frames_w_target > FRAMES_HELD_THRESH:
                        frames_w_target = 0
                        closest_person_x = closest_face.keypoints[2].x
                        state = State.CENTER
                
                
            # camera hones toward closest person to the left/right of center (direction dependent)
            case State.CENTER:
                # find closest target determined in search phase
                target_face = find_target(fd_result.detections, closest_person_x)
                
                # if nobody is there, go back to search phase
                if target_face is None:
                    frames_w_no_target += 1
                    
                    if frames_w_no_target > FRAMES_HELD_THRESH:
                        frames_w_no_target = 0
                        state = State.SEARCH
                
                else:
                    # center of target person
                    person_center = target_face.keypoints[2].x
                    closest_person_x = person_center
                        
                    # tell MCU what direction to turn to center output
                    if person_center < 0.5 - CENTER_MARGIN:
                        right = 1
                        
                    elif person_center > 0.5 + CENTER_MARGIN:
                        left = 1
                        
                    else:
                        # start timer for authentication
                        confirm_start = time.monotonic()
                        center = 1
                        frames_w_target_centered += 1
                
                    if frames_w_target_centered > FRAMES_HELD_THRESH:
                        state = State.CONFIRM
                
                
            # scan for nearby thumbs, and if a thumb is held for long enough then they're a player
            # otherwise if the timer runs past a set time then they're not a player
            case State.CONFIRM:
                center = 1
                # scan for thumbs
                for gesture, hand in zip(gr_result.gestures, gr_result.hand_landmarks):
                    if gesture and gesture[0].category_name == "Thumb_Up":
                        thumb = hand[4]
                        
                        # record a thumb whose x value is within the margin
                        if abs(person_center - thumb.x) < CONFIRM_MARGIN:
                            frames_w_thumbs_up += 1
                            break
                    
                # transition if thumb is held close to nose (horizontally) long enough
                if frames_w_thumbs_up > FRAMES_HELD_THRESH:
                    person_type = "PLAYER"
                    state = State.WAIT
                
                # bystander on timeout
                elif time.monotonic() - confirm_start > CONFIRM_TIMEOUT:
                    person_type = "BYSTANDER"
                    state = State.WAIT


            # report player
            case State.WAIT:
                if person_type == "PLAYER":
                    player = 1
                elif person_type == "BYSTANDER":
                    skip = 1
                    
                if reset:
                    state = State.IDLE
                
        
        # assign output values
        out_right.value = right
        out_left.value = left
        out_center.value = center
        out_player.value = player
        out_skip.value = skip
        
        
        # print output signals 
        cv2.putText(frame, f"state: {state}   status:{person_type}    reset:{reset}    dir:{direction}", (10, 25),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)
        cv2.imshow("center your face", frame)
        
        key = cv2.waitKey(1) & 0xFF
        if key == ord("q"):
            break
        
        #DEBUGGING
        if key == ord("r"):
            inp_reset = not inp_reset
            
        if key == ord("d"):
            inp_direction = not inp_direction
            

cam.stop()
cv2.destroyAllWindows()
