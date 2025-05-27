// Created by Crowbar 0.71

$modelname "player/Chell_animations.mdl"

$surfaceprop "default"

$contents "solid"

$illumposition -2.811 -0.781 32.006

$cdmaterials "models\player\chell\"

$attachment "eyes" "head" 0 3.42 3.91 rotate -90 -90 0
$attachment "forward" "forward" 0 0 0 rotate 0 0 0
$attachment "righteye" "head" -1.31 3.42 3.91 rotate -90 -90 0
$attachment "lefteye" "head" 1.31 3.42 3.91 rotate -90 -90 0

$cbox 0 0 0 0 0 0

$bbox -15.617 -9.184 -0.674 17.179 3.562 64.687

$definebone "root" "" 0 43.329243 0.634114 0 0 0 0 0 0 0 0 0
$definebone "hips" "root" 0 0 0 0 0 0 0 0 0 0 0 0
$definebone "thigh_L" "hips" 4.350457 -2.452274 -0.220442 14.713023 1.870061 171.17245 0 0 0 0 0 0
$definebone "thigh_twist_L" "thigh_L" 0 8.333012 0.000001 0 90.000009 90.000016 0 0 0 0 0 0
$definebone "knee_L" "thigh_twist_L" 8.333015 0 0 0.000001 23.638699 90.000009 0 0 0 0 0 0
$definebone "ankle_L" "knee_L" 18.142759 0 0 -13.442373 91.309418 152.88399 0 0 0 0 0 0
$definebone "toe_L" "ankle_L" 0.003752 6.164958 0.152068 1.41343 0.000001 -77.546566 0 0 0 0 0 0
$definebone "spring_base_L" "knee_L" 4.617707 0.531995 -2.490242 -11.126517 91.012735 179.177596 0 0 0 0 0 0
$definebone "spring_end_L" "spring_base_L" 0 20.883545 0 -1.427757 0.350419 166.208774 0 0 0 0 0 0
$definebone "spring_1_L" "spring_base_L" -0.129009 18.597668 4.167802 -2.588783 0.212534 163.093566 0 0 0 0 0 0
$definebone "thigh_R" "hips" -4.35046 -2.45224 -0.220441 -14.713023 -1.870061 -8.827551 0 0 0 0 0 0
$definebone "thigh_twist_R" "thigh_R" 0.000003 -8.333091 -0.000017 -0.000015 -90.000009 -0.000003 0 0 0 0 0 0
$definebone "knee_R" "thigh_twist_R" 8.332994 0 0 23.638704 0.000008 90.000009 0 0 0 0 0 0
$definebone "ankle_R" "knee_R" 18.142736 0.000001 0 76.495168 -84.538658 158.499649 0 0 0 0 0 0
$definebone "toe_R" "ankle_R" -0.003752 -6.164957 -0.152072 1.413428 0 -77.546566 0 0 0 0 0 0
$definebone "spring_base_R" "knee_R" 4.617711 -2.490237 0.531992 78.828074 -84.864704 -175.588468 0 0 0 0 0 0
$definebone "spring_end_R" "spring_base_R" -0.000005 -20.88352 -0.000004 -1.427759 0.350419 166.208774 0 0 0 0 0 0
$definebone "spring_1_R" "spring_base_R" 0.129005 -18.597641 -4.167804 -2.588785 0.212533 163.093566 0 0 0 0 0 0
$definebone "spine_base" "root" 0 3.918041 0.107099 0 0 3.237073 0 0 0 0 0 0
$definebone "spine_mid" "spine_base" 0 3.79328 0 0 0 -9.888406 0 0 0 0 0 0
$definebone "chest" "spine_mid" 0 4.392048 0 0 0 -4.049068 0 0 0 0 0 0
$definebone "neck" "chest" 0 7.387572 -0.000001 0 0 24.376964 0 0 0 0 0 0
$definebone "head" "neck" 0 3.276221 0 0 0 -13.676565 0 0 0 0 0 0
$definebone "clavicle_L" "chest" 0.89586 3.917912 0.656975 0 0 10.700402 0 0 0 0 0 0
$definebone "bicep_L" "clavicle_L" 4.123603 -0.117653 -2.440016 -1.261874 -43.917648 178.738428 0 0 0 0 0 0
$definebone "bicep_twist_L" "bicep_L" 4.900001 0.000001 0 -1.261569 90.000023 180.000005 0 0 0 0 0 0
$definebone "elbow_L" "bicep_L" 9.799999 0 0 13.104296 0.293722 -88.704698 0 0 0 0 0 0
$definebone "wrist_twist_L" "elbow_L" 4.900002 0 -0.000002 -89.999989 -90.000016 0 0 0 0 0 0 0
$definebone "wrist_L" "elbow_L" 9.8 -0.000001 -0.000003 -89.999989 -90.000016 0 0 0 0 0 0 0
$definebone "thumb_base_L" "wrist_L" -0.187864 1.241705 1.202023 38.992276 59.732777 72.362726 0 0 0 0 0 0
$definebone "thumb_mid_L" "thumb_base_L" 0.000001 1.590449 -0.000001 0.000003 -2.791481 -0.000007 0 0 0 0 0 0
$definebone "thumb_end_L" "thumb_mid_L" 0 1.179849 0.000001 0.000004 13.408064 -0.000006 0 0 0 0 0 0
$definebone "index_base_L" "wrist_L" -0.942188 4.324296 1.306554 -5.626065 1.877454 2.87605 0 0 0 0 0 0
$definebone "index_mid_L" "index_base_L" 0.000001 1.287341 0 -0.000002 -14.714625 -0.000004 0 0 0 0 0 0
$definebone "index_end_L" "index_mid_L" 0 1.044394 0 -0.000001 -4.685691 -0.000006 0 0 0 0 0 0
$definebone "mid_base_L" "wrist_L" -0.6626 4.455425 0.385188 -17.683942 5.892825 -3.351226 0 0 0 0 0 0
$definebone "mid_mid_L" "mid_base_L" -0.000002 1.560847 0 0.000004 -8.809289 0 0 0 0 0 0 0
$definebone "mid_end_L" "mid_mid_L" -0.000002 1.05259 0.000001 0.000003 -1.543041 0 0 0 0 0 0 0
$definebone "ring_base_L" "wrist_L" -0.259947 4.274167 -0.539759 -22.573767 3.079179 -7.509109 0 0 0 0 0 0
$definebone "ring_mid_L" "ring_base_L" 0 1.433729 0 0.000008 -7.020355 0 0 0 0 0 0 0
$definebone "ring_end_L" "ring_mid_L" 0.000003 1.052591 -0.000001 0.000008 -0.000002 0 0 0 0 0 0 0
$definebone "pinky_base_L" "wrist_L" 0.14135 3.871773 -1.363274 -40.093596 0.535897 -21.794157 0 0 0 0 0 0
$definebone "pinky_mid_L" "pinky_base_L" -0.000003 1.065791 0 0.000003 -7.020359 0 0 0 0 0 0 0
$definebone "pinky_end_L" "pinky_mid_L" 0.000001 0.779176 0 0.000003 -0.000004 -0.000001 0 0 0 0 0 0
$definebone "clavicle_R" "chest" -0.89586 3.917945 0.656981 0 0 -169.299598 0 0 0 0 0 0
$definebone "bicep_R" "clavicle_R" -4.1236 0.117698 2.440021 1.261888 136.082394 1.26157 0 0 0 0 0 0
$definebone "bicep_twist_R" "bicep_R" 4.899981 0.000021 -0.000002 1.261569 89.999948 -0.000014 0 0 0 0 0 0
$definebone "elbow_R" "bicep_R" 9.800055 0.000002 0 13.10441 -0.293774 -1.295306 0 0 0 0 0 0
$definebone "wrist_twist_R" "elbow_R" 4.89996 0 0 -0.000001 90.000009 -0.000125 0 0 0 0 0 0
$definebone "wrist_R" "elbow_R" 9.799915 0.000002 0 -0.000001 90.000009 -0.000125 0 0 0 0 0 0
$definebone "thumb_base_R" "wrist_R" 0.187868 -1.241719 -1.20202 38.992276 59.732777 72.362726 0 0 0 0 0 0
$definebone "thumb_mid_R" "thumb_base_R" 0.000037 -1.590426 -0.000016 0.000003 -2.791485 -0.000008 0 0 0 0 0 0
$definebone "thumb_end_R" "thumb_mid_R" -0.00007 -1.179891 0.000028 0.000006 13.408063 -0.000009 0 0 0 0 0 0
$definebone "index_base_R" "wrist_R" 0.942218 -4.324327 -1.30655 -5.626066 1.87745 2.876052 0 0 0 0 0 0
$definebone "index_mid_R" "index_base_R" -0.000016 -1.287268 -0.000011 -0.000002 -14.71463 -0.000001 0 0 0 0 0 0
$definebone "index_end_R" "index_mid_R" 0.000041 -1.044481 0.00003 -0.000001 -4.685692 0 0 0 0 0 0 0
$definebone "mid_base_R" "wrist_R" 0.662647 -4.455462 -0.385182 -17.683942 5.892821 -3.351225 0 0 0 0 0 0
$definebone "mid_mid_R" "mid_base_R" -0.000047 -1.560865 0.000031 0.000003 -8.809291 0.000002 0 0 0 0 0 0
$definebone "mid_end_R" "mid_mid_R" -0.000002 -1.052544 -0.000011 0.000004 -1.543037 0.000002 0 0 0 0 0 0
$definebone "ring_base_R" "wrist_R" 0.259921 -4.274165 0.53976 -22.573767 3.079176 -7.509108 0 0 0 0 0 0
$definebone "ring_mid_R" "ring_base_R" 0.000076 -1.433766 -0.000023 0.000007 -7.020357 0.000002 0 0 0 0 0 0
$definebone "ring_end_R" "ring_mid_R" -0.000042 -1.052585 0.000007 0.000007 -0.000002 0.000002 0 0 0 0 0 0
$definebone "pinky_base_R" "wrist_R" -0.141325 -3.871754 1.36327 -40.0936 0.535895 -21.794157 0 0 0 0 0 0
$definebone "pinky_mid_R" "pinky_base_R" -0.00001 -1.065874 0.000004 0 -7.020361 0.000004 0 0 0 0 0 0
$definebone "pinky_end_R" "pinky_mid_R" 0.000023 -0.779119 -0.000022 0.000001 -0.000007 0.000004 0 0 0 0 0 0
$definebone "weapon_bone" "wrist_R" 0.325557 0.019277 0.438602 -78.99999 180.000005 90.000016 0 0 0 0 0 0
$definebone "forward" "head" 0 2.500004 0 -90.000003 -90.000003 0 0 0 0 0 0 0


$sectionframes 30 160

$poseparameter "body_pitch" 45 -45 loop 360
$poseparameter "body_yaw" -45 45 loop 360
$poseparameter "head_yaw" -29.28032 29.24085 loop 0
$poseparameter "head_pitch" -20.75119 21.41426 loop 0
$poseparameter "move_yaw" -180 180 wrap

$ikchain "rhand" "wrist_R" knee 0 0 0
$ikchain "lhand" "wrist_L" knee 0 0 0
$ikchain "rfoot" "ankle_R" knee 0.273 0.008 0.962
$ikchain "lfoot" "ankle_L" knee 0.195 -0.953 0.233

$weightlist "weights_head_rot_z" {
	 "root" 0
	 "hips" 0
	 "thigh_L" 0
	 "thigh_twist_L" 0
	 "knee_L" 0
	 "ankle_L" 0
	 "toe_L" 0
	 "spring_base_L" 0
	 "spring_end_L" 0
	 "spring_1_L" 0
	 "thigh_R" 0
	 "thigh_twist_R" 0
	 "knee_R" 0
	 "ankle_R" 0
	 "toe_R" 0
	 "spring_base_R" 0
	 "spring_end_R" 0
	 "spring_1_R" 0
	 "spine_base" 0
	 "spine_mid" 1
	 "chest" 1
	 "neck" 1
	 "head" 1
	 "clavicle_L" 0
	 "bicep_L" 0
	 "bicep_twist_L" 0
	 "elbow_L" 0
	 "wrist_twist_L" 0
	 "wrist_L" 0
	 "thumb_base_L" 0
	 "thumb_mid_L" 0
	 "thumb_end_L" 0
	 "index_base_L" 0
	 "index_mid_L" 0
	 "index_end_L" 0
	 "mid_base_L" 0
	 "mid_mid_L" 0
	 "mid_end_L" 0
	 "ring_base_L" 0
	 "ring_mid_L" 0
	 "ring_end_L" 0
	 "pinky_base_L" 0
	 "pinky_mid_L" 0
	 "pinky_end_L" 0
	 "clavicle_R" 0
	 "bicep_R" 0
	 "bicep_twist_R" 0
	 "elbow_R" 0
	 "wrist_twist_R" 0
	 "wrist_R" 0
	 "thumb_base_R" 0
	 "thumb_mid_R" 0
	 "thumb_end_R" 0
	 "index_base_R" 0
	 "index_mid_R" 0
	 "index_end_R" 0
	 "mid_base_R" 0
	 "mid_mid_R" 0
	 "mid_end_R" 0
	 "ring_base_R" 0
	 "ring_mid_R" 0
	 "ring_end_R" 0
	 "pinky_base_R" 0
	 "pinky_mid_R" 0
	 "pinky_end_R" 0
	 "weapon_bone" 0
	 "forward" 1
}

$weightlist "weights_portal_aimmatrix" {
	 "root" 0
	 "hips" 0
	 "thigh_L" 0
	 "thigh_twist_L" 0
	 "knee_L" 0
	 "ankle_L" 0
	 "toe_L" 0
	 "spring_base_L" 0
	 "spring_end_L" 0
	 "spring_1_L" 0
	 "thigh_R" 0
	 "thigh_twist_R" 0
	 "knee_R" 0
	 "ankle_R" 0
	 "toe_R" 0
	 "spring_base_R" 0
	 "spring_end_R" 0
	 "spring_1_R" 0
	 "spine_base" 1
	 "spine_mid" 1
	 "chest" 1
	 "neck" 1
	 "head" 1
	 "clavicle_L" 1
	 "bicep_L" 1
	 "bicep_twist_L" 1
	 "elbow_L" 1
	 "wrist_twist_L" 1
	 "wrist_L" 1
	 "thumb_base_L" 1
	 "thumb_mid_L" 1
	 "thumb_end_L" 1
	 "index_base_L" 1
	 "index_mid_L" 1
	 "index_end_L" 1
	 "mid_base_L" 1
	 "mid_mid_L" 1
	 "mid_end_L" 1
	 "ring_base_L" 1
	 "ring_mid_L" 1
	 "ring_end_L" 1
	 "pinky_base_L" 1
	 "pinky_mid_L" 1
	 "pinky_end_L" 1
	 "clavicle_R" 1
	 "bicep_R" 1
	 "bicep_twist_R" 1
	 "elbow_R" 1
	 "wrist_twist_R" 1
	 "wrist_R" 1
	 "thumb_base_R" 1
	 "thumb_mid_R" 1
	 "thumb_end_R" 1
	 "index_base_R" 1
	 "index_mid_R" 1
	 "index_end_R" 1
	 "mid_base_R" 1
	 "mid_mid_R" 1
	 "mid_end_R" 1
	 "ring_base_R" 1
	 "ring_mid_R" 1
	 "ring_end_R" 1
	 "pinky_base_R" 1
	 "pinky_mid_R" 1
	 "pinky_end_R" 1
	 "weapon_bone" 1
	 "forward" 1
}

$animation "a_head_rot_z_lt_corrective_animation" "chell_animations_anims\a_head_rot_z_lt_corrective_animation.smd" {
}

$animation "a_head_rot_corrective_animation" "chell_animations_anims\a_head_rot_corrective_animation.smd" {
}

$animation "a_head_rot_z_rt_corrective_animation" "chell_animations_anims\a_head_rot_z_rt_corrective_animation.smd" {
}

$animation "a_head_rot_dwn_corrective_animation" "chell_animations_anims\a_head_rot_dwn_corrective_animation.smd" {
}

$animation "a_head_rot_up_corrective_animation" "chell_animations_anims\a_head_rot_up_corrective_animation.smd" {
}

$animation "a_portal_aimmatrix_up_right_corrective_animation" "chell_animations_anims\a_portal_aimmatrix_up_right_corrective_animation.smd" {
}

$animation "a_portal_aimmatrix_up_center_corrective_animation" "chell_animations_anims\a_portal_aimmatrix_up_center_corrective_animation.smd" {
}

$animation "a_portal_aimmatrix_up_left_corrective_animation" "chell_animations_anims\a_portal_aimmatrix_up_left_corrective_animation.smd" {
}

$animation "a_portal_aimmatrix_mid_right_corrective_animation" "chell_animations_anims\a_portal_aimmatrix_mid_right_corrective_animation.smd" {
}

$animation "a_portal_aimmatrix_mid_center_corrective_animation" "chell_animations_anims\a_portal_aimmatrix_mid_center_corrective_animation.smd" {
}

$animation "a_portal_aimmatrix_mid_left_corrective_animation" "chell_animations_anims\a_portal_aimmatrix_mid_left_corrective_animation.smd" {
}

$animation "a_portal_aimmatrix_down_right_corrective_animation" "chell_animations_anims\a_portal_aimmatrix_down_right_corrective_animation.smd" {
}

$animation "a_portal_aimmatrix_down_center_corrective_animation" "chell_animations_anims\a_portal_aimmatrix_down_center_corrective_animation.smd" {
}

$animation "a_portal_aimmatrix_down_left_corrective_animation" "chell_animations_anims\a_portal_aimmatrix_down_left_corrective_animation.smd" {
}

$animation "a_run_aimmatrix_up_right_corrective_animation" "chell_animations_anims\a_run_aimmatrix_up_right_corrective_animation.smd" {
}

$animation "a_run_aimmatrix_up_center_corrective_animation" "chell_animations_anims\a_run_aimmatrix_up_center_corrective_animation.smd" {
}

$animation "a_run_aimmatrix_up_left_corrective_animation" "chell_animations_anims\a_run_aimmatrix_up_left_corrective_animation.smd" {
}

$animation "a_run_aimmatrix_mid_right_corrective_animation" "chell_animations_anims\a_run_aimmatrix_mid_right_corrective_animation.smd" {
}

$animation "a_run_aimmatrix_mid_center_corrective_animation" "chell_animations_anims\a_run_aimmatrix_mid_center_corrective_animation.smd" {
}

$animation "a_run_aimmatrix_mid_left_corrective_animation" "chell_animations_anims\a_run_aimmatrix_mid_left_corrective_animation.smd" {
}

$animation "a_run_aimmatrix_down_right_corrective_animation" "chell_animations_anims\a_run_aimmatrix_down_right_corrective_animation.smd" {
}

$animation "a_run_aimmatrix_down_center_corrective_animation" "chell_animations_anims\a_run_aimmatrix_down_center_corrective_animation.smd" {
}

$animation "a_run_aimmatrix_down_left_corrective_animation" "chell_animations_anims\a_run_aimmatrix_down_left_corrective_animation.smd" {
}

$animation "a_crouch_aimmatrix_up_right_corrective_animation" "chell_animations_anims\a_crouch_aimmatrix_up_right_corrective_animation.smd" {
}

$animation "a_crouch_aimmatrix_up_center_corrective_animation" "chell_animations_anims\a_crouch_aimmatrix_up_center_corrective_animation.smd" {
}

$animation "a_crouch_aimmatrix_up_left_corrective_animation" "chell_animations_anims\a_crouch_aimmatrix_up_left_corrective_animation.smd" {
}

$animation "a_crouch_aimmatrix_mid_right_corrective_animation" "chell_animations_anims\a_crouch_aimmatrix_mid_right_corrective_animation.smd" {
}

$animation "a_crouch_aimmatrix_mid_center_corrective_animation" "chell_animations_anims\a_crouch_aimmatrix_mid_center_corrective_animation.smd" {
}

$animation "a_crouch_aimmatrix_mid_left_corrective_animation" "chell_animations_anims\a_crouch_aimmatrix_mid_left_corrective_animation.smd" {
}

$animation "a_crouch_aimmatrix_down_right_corrective_animation" "chell_animations_anims\a_crouch_aimmatrix_down_right_corrective_animation.smd" {
}

$animation "a_crouch_aimmatrix_down_center_corrective_animation" "chell_animations_anims\a_crouch_aimmatrix_down_center_corrective_animation.smd" {
}

$animation "a_crouch_aimmatrix_down_left_corrective_animation" "chell_animations_anims\a_crouch_aimmatrix_down_left_corrective_animation.smd" {
}

$animation "a_stand_fire_PORTALGUN_corrective_animation" "chell_animations_anims\a_stand_fire_PORTALGUN_corrective_animation.smd" {
}

$animation "a_crouch_fire_PORTALGUN_corrective_animation" "chell_animations_anims\a_crouch_fire_PORTALGUN_corrective_animation.smd" {
}

$animation "a_reference" "chell_animations_anims\a_reference.smd" {
	fps 30
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 160 160 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 160 160 target 1
	ikrule "rfoot" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 160 160 target 2
	ikrule "lfoot" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 160 160 target 3
	walkframe 151 LX LY
}

$animation "a_pose_neutral" "chell_animations_anims\a_pose_neutral.smd" {
	fps 30
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 0 0 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 0 0 target 1
	ikrule "rfoot" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 0 0 target 2
	ikrule "lfoot" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 0 0 target 3
}

$animation "a_head_rot" "chell_animations_anims\a_head_rot.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_head_rot_corrective_animation" 0
	weightlist "weights_head_rot_z"
}

$animation "a_head_rot_lt" "chell_animations_anims\a_head_rot_lt.smd" {
	fps 30
}

$animation "a_head_rot_rt" "chell_animations_anims\a_head_rot_rt.smd" {
	fps 30
}

$animation "a_head_rot_dwn" "chell_animations_anims\a_head_rot_dwn.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_head_rot_dwn_corrective_animation" 0
	weightlist "weights_head_rot_z"
}

$animation "a_head_rot_up" "chell_animations_anims\a_head_rot_up.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_head_rot_up_corrective_animation" 0
	weightlist "weights_head_rot_z"
}

$animation "a_head_rot_z_rt" "chell_animations_anims\a_head_rot_z_rt.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_head_rot_z_rt_corrective_animation" 0
	weightlist "weights_head_rot_z"
}

$animation "a_head_rot_z_lt" "chell_animations_anims\a_head_rot_z_lt.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_head_rot_z_lt_corrective_animation" 0
	weightlist "weights_head_rot_z"
}

$animation "a_portal_aimmatrix_neutral" "chell_animations_anims\a_portal_aimmatrix_neutral.smd" {
	fps 30
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 0 0 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 0 0 target 1
}

$animation "a_portal_aimmatrix_down_right" "chell_animations_anims\a_portal_aimmatrix_down_right.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_portal_aimmatrix_down_right_corrective_animation" 0
	weightlist "weights_portal_aimmatrix"
}

$animation "a_portal_aimmatrix_down_center" "chell_animations_anims\a_portal_aimmatrix_down_center.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_portal_aimmatrix_down_center_corrective_animation" 0
	weightlist "weights_portal_aimmatrix"
}

$animation "a_portal_aimmatrix_down_left" "chell_animations_anims\a_portal_aimmatrix_down_left.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_portal_aimmatrix_down_left_corrective_animation" 0
	weightlist "weights_portal_aimmatrix"
}

$animation "a_portal_aimmatrix_mid_right" "chell_animations_anims\a_portal_aimmatrix_mid_right.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_portal_aimmatrix_mid_right_corrective_animation" 0
	weightlist "weights_portal_aimmatrix"
}

$animation "a_portal_aimmatrix_mid_center" "chell_animations_anims\a_portal_aimmatrix_mid_center.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_portal_aimmatrix_mid_center_corrective_animation" 0
	weightlist "weights_portal_aimmatrix"
}

$animation "a_portal_aimmatrix_mid_left" "chell_animations_anims\a_portal_aimmatrix_mid_left.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_portal_aimmatrix_mid_left_corrective_animation" 0
	weightlist "weights_portal_aimmatrix"
}

$animation "a_portal_aimmatrix_up_right" "chell_animations_anims\a_portal_aimmatrix_up_right.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_portal_aimmatrix_up_right_corrective_animation" 0
	weightlist "weights_portal_aimmatrix"
}

$animation "a_portal_aimmatrix_up_center" "chell_animations_anims\a_portal_aimmatrix_up_center.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_portal_aimmatrix_up_center_corrective_animation" 0
	weightlist "weights_portal_aimmatrix"
}

$animation "a_portal_aimmatrix_up_left" "chell_animations_anims\a_portal_aimmatrix_up_left.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_portal_aimmatrix_up_left_corrective_animation" 0
	weightlist "weights_portal_aimmatrix"
}

$animation "a_portal_aimmatrix_straight_up" "chell_animations_anims\a_portal_aimmatrix_straight_up.smd" {
	fps 30
}

$animation "a_run_aimmatrix_neutral" "chell_animations_anims\a_run_aimmatrix_neutral.smd" {
	fps 30
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 0 0 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 0 0 target 1
}

$animation "a_run_aimmatrix_down_right" "chell_animations_anims\a_run_aimmatrix_down_right.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_run_aimmatrix_down_right_corrective_animation" 0
	weightlist "weights_portal_aimmatrix"
}

$animation "a_run_aimmatrix_down_center" "chell_animations_anims\a_run_aimmatrix_down_center.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_run_aimmatrix_down_center_corrective_animation" 0
	weightlist "weights_portal_aimmatrix"
}

$animation "a_run_aimmatrix_down_left" "chell_animations_anims\a_run_aimmatrix_down_left.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_run_aimmatrix_down_left_corrective_animation" 0
	weightlist "weights_portal_aimmatrix"
}

$animation "a_run_aimmatrix_mid_right" "chell_animations_anims\a_run_aimmatrix_mid_right.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_run_aimmatrix_mid_right_corrective_animation" 0
	weightlist "weights_portal_aimmatrix"
}

$animation "a_run_aimmatrix_mid_center" "chell_animations_anims\a_run_aimmatrix_mid_center.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_run_aimmatrix_mid_center_corrective_animation" 0
	weightlist "weights_portal_aimmatrix"
}

$animation "a_run_aimmatrix_mid_left" "chell_animations_anims\a_run_aimmatrix_mid_left.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_run_aimmatrix_mid_left_corrective_animation" 0
	weightlist "weights_portal_aimmatrix"
}

$animation "a_run_aimmatrix_up_right" "chell_animations_anims\a_run_aimmatrix_up_right.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_run_aimmatrix_up_right_corrective_animation" 0
	weightlist "weights_portal_aimmatrix"
}

$animation "a_run_aimmatrix_up_center" "chell_animations_anims\a_run_aimmatrix_up_center.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_run_aimmatrix_up_center_corrective_animation" 0
	weightlist "weights_portal_aimmatrix"
}

$animation "a_run_aimmatrix_up_left" "chell_animations_anims\a_run_aimmatrix_up_left.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_run_aimmatrix_up_left_corrective_animation" 0
	weightlist "weights_portal_aimmatrix"
}

$animation "a_run_aimmatrix_straight_up" "chell_animations_anims\a_run_aimmatrix_straight_up.smd" {
	fps 30
}

$animation "a_crouch_aimmatrix_neutral" "chell_animations_anims\a_crouch_aimmatrix_neutral.smd" {
	fps 30
	ikrule "lhand" touch "root" contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 0 0 target 1
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 0 0 target 0
}

$animation "a_crouch_aimmatrix_down_right" "chell_animations_anims\a_crouch_aimmatrix_down_right.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_crouch_aimmatrix_down_right_corrective_animation" 0
	ikrule "lhand" touch "root" contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 0 0 target 1
	weightlist "weights_portal_aimmatrix"
}

$animation "a_crouch_aimmatrix_down_center" "chell_animations_anims\a_crouch_aimmatrix_down_center.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_crouch_aimmatrix_down_center_corrective_animation" 0
	ikrule "lhand" touch "root" contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 0 0 target 1
	weightlist "weights_portal_aimmatrix"
}

$animation "a_crouch_aimmatrix_down_left" "chell_animations_anims\a_crouch_aimmatrix_down_left.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_crouch_aimmatrix_down_left_corrective_animation" 0
	ikrule "lhand" touch "root" contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 0 0 target 1
	weightlist "weights_portal_aimmatrix"
}

$animation "a_crouch_aimmatrix_mid_right" "chell_animations_anims\a_crouch_aimmatrix_mid_right.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_crouch_aimmatrix_mid_right_corrective_animation" 0
	ikrule "lhand" touch "root" contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 0 0 target 1
	weightlist "weights_portal_aimmatrix"
}

$animation "a_crouch_aimmatrix_mid_center" "chell_animations_anims\a_crouch_aimmatrix_mid_center.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_crouch_aimmatrix_mid_center_corrective_animation" 0
	ikrule "lhand" touch "root" contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 0 0 target 1
	weightlist "weights_portal_aimmatrix"
}

$animation "a_crouch_aimmatrix_mid_left" "chell_animations_anims\a_crouch_aimmatrix_mid_left.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_crouch_aimmatrix_mid_left_corrective_animation" 0
	ikrule "lhand" touch "root" contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 0 0 target 1
	weightlist "weights_portal_aimmatrix"
}

$animation "a_crouch_aimmatrix_up_right" "chell_animations_anims\a_crouch_aimmatrix_up_right.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_crouch_aimmatrix_up_right_corrective_animation" 0
	ikrule "lhand" touch "root" contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 0 0 target 1
	weightlist "weights_portal_aimmatrix"
}

$animation "a_crouch_aimmatrix_up_center" "chell_animations_anims\a_crouch_aimmatrix_up_center.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_crouch_aimmatrix_up_center_corrective_animation" 0
	ikrule "lhand" touch "root" contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 0 0 target 1
	weightlist "weights_portal_aimmatrix"
}

$animation "a_crouch_aimmatrix_up_left" "chell_animations_anims\a_crouch_aimmatrix_up_left.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_crouch_aimmatrix_up_left_corrective_animation" 0
	ikrule "lhand" touch "root" contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 0 0 target 1
	weightlist "weights_portal_aimmatrix"
}

$animation "a_crouch_aimmatrix_straight_up" "chell_animations_anims\a_crouch_aimmatrix_straight_up.smd" {
	fps 30
	ikrule "lhand" touch "root" contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 0 0 target 1
}

$animation "a_stand_fire_PORTALGUN" "chell_animations_anims\a_stand_fire_PORTALGUN.smd" {
	fps 30
	loop
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_stand_fire_PORTALGUN_corrective_animation" 0
	ikrule "lfoot" footstep contact 1 fakeorigin 14.23 0.59 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin -7.25 1.63 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 2
	walkframe 25 LX LY
}

$animation "a_crouch_fire_PORTALGUN" "chell_animations_anims\a_crouch_fire_PORTALGUN.smd" {
	fps 30
	loop
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_crouch_fire_PORTALGUN_corrective_animation" 0
	ikrule "lfoot" footstep contact 1 fakeorigin 14.2 0.44 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin -7.2 2.29 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 2
	walkframe 25 LX LY
}

$animation "a_idle_PORTALGUN" "chell_animations_anims\a_idle_PORTALGUN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 14.28 0.49 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 81 160 2 81 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin -7.28 1.86 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 81 160 2 81 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 160 160 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 160 160 target 1
	walkframe 160 LX LY
}

$animation "a_idle" "chell_animations_anims\a_idle.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 12.88 0.86 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 161 320 2 161 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin -7.62 1.19 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 161 320 2 161 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 320 320 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 320 320 target 1
	walkframe 320 LX LY
}

$animation "a_CROUCHidle_PORTALGUN" "chell_animations_anims\a_CROUCHidle_PORTALGUN.smd" {
	fps 30
	loop
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 160 160 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 160 160 target 1
	ikrule "rfoot" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 160 160 target 2
	ikrule "lfoot" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 160 160 target 3
	walkframe 160 LX LY
}

$animation "a_CrouchIdle" "chell_animations_anims\a_CrouchIdle.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 12.07 -0.38 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin -0.28 -5.51 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1
	walkframe 25 LX LY
}

$animation "a_runCenter_PORTALGUN" "chell_animations_anims\a_runCenter_PORTALGUN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 12.86 0.86 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 3
	ikrule "rfoot" footstep contact 0 fakeorigin -7.59 1.19 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 0 0 0 0 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 1
}

$animation "a_runS_PORTALGUN" "chell_animations_anims\a_runS_PORTALGUN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 14.05 0.9 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 3
	ikrule "rfoot" footstep contact 0 fakeorigin 5.47 -2.63 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 0 0 0 0 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 1
	walkframe 30 LX LY
}

$animation "a_runSE_PORTALGUN" "chell_animations_anims\a_runSE_PORTALGUN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 14.3 8.72 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 3
	ikrule "rfoot" footstep contact 0 fakeorigin 5.37 -15.27 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 0 0 0 0 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 1
	walkframe 30 LX LY
}

$animation "a_runE_PORTALGUN" "chell_animations_anims\a_runE_PORTALGUN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 4.05 16.53 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin -12.36 -2.47 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 1
	walkframe 30 LX LY
}

$animation "a_runNE_PORTALGUN" "chell_animations_anims\a_runNE_PORTALGUN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin -11.79 4.88 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin 4.83 -13.36 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 1
	walkframe 30 LX LY
}

$animation "a_runN_PORTALGUN" "chell_animations_anims\a_runN_PORTALGUN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin -5.05 0.77 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin -2.77 -1.74 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 1
	walkframe 30 LX LY
}

$animation "a_runNW_PORTALGUN" "chell_animations_anims\a_runNW_PORTALGUN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 4.92 3.33 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin 3.46 -9.8 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 1
	walkframe 30 LX LY
}

$animation "a_runW_PORTALGUN" "chell_animations_anims\a_runW_PORTALGUN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin -5.75 2.42 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin 13.11 -7.17 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 1
	walkframe 30 LX LY
}

$animation "a_runSW_PORTALGUN" "chell_animations_anims\a_runSW_PORTALGUN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin -6.27 -1.49 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin -0.25 -7.83 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 1
	walkframe 30 LX LY
}

$animation "a_runCenter" "chell_animations_anims\a_runCenter.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 12.86 0.86 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 3
	ikrule "rfoot" footstep contact 0 fakeorigin -7.59 1.19 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 0 0 0 0 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 1
}

$animation "a_runS" "chell_animations_anims\a_runS.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 13.94 0.89 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 3
	ikrule "rfoot" footstep contact 0 fakeorigin 6.36 -2.87 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 0 0 0 0 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 1
	walkframe 30 LX LY
}

$animation "a_runSE" "chell_animations_anims\a_runSE.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 14.19 8.67 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 3
	ikrule "rfoot" footstep contact 0 fakeorigin 5.37 -15.27 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 0 0 0 0 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 1
	walkframe 30 LX LY
}

$animation "a_runE" "chell_animations_anims\a_runE.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 8.86 16.45 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin -13.25 -7.79 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 1
	walkframe 30 LX LY
}

$animation "a_runNE" "chell_animations_anims\a_runNE.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin -12.54 4.98 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin 3.18 -13.36 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 1
	walkframe 30 LX LY
}

$animation "a_runN" "chell_animations_anims\a_runN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin -5.05 0.77 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin -2.77 -1.74 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 1
	walkframe 30 LX LY
}

$animation "a_runNW" "chell_animations_anims\a_runNW.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin -2.78 8.74 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin -3.68 -7.95 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 1
	walkframe 30 LX LY
}

$animation "a_runW" "chell_animations_anims\a_runW.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin -5.45 1.51 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin 10.32 -13.05 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 1
	walkframe 30 LX LY
}

$animation "a_runSW" "chell_animations_anims\a_runSW.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin -6.27 -1.49 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin -0.25 -7.83 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 16 30 2 16 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 30 30 target 1
	walkframe 30 LX LY
}

$animation "a_crouchCenter_PORTALGUN" "chell_animations_anims\a_crouchCenter_PORTALGUN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 22.53 -0.29 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 0 fakeorigin -14.94 -4.03 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 0 0 0 0 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1
	walkframe 25 LX LY
}

$animation "a_crouchS_PORTALGUN" "chell_animations_anims\a_crouchS_PORTALGUN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 7.83 -0.33 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin -0.55 -4.19 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1
	walkframe 25 LX LY
}

$animation "a_crouchSE_PORTALGUN" "chell_animations_anims\a_crouchSE_PORTALGUN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 8.63 0.73 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin 5.5 -9.95 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1
	walkframe 25 LX LY
}

$animation "a_crouchE_PORTALGUN" "chell_animations_anims\a_crouchE_PORTALGUN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 15.54 -1.94 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin 4.83 -14.71 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1
	walkframe 25 LX LY
}

$animation "a_crouchNE_PORTALGUN" "chell_animations_anims\a_crouchNE_PORTALGUN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 9.99 0.11 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin 2.63 -6.22 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1
	walkframe 25 LX LY
}

$animation "a_crouchN_PORTALGUN" "chell_animations_anims\a_crouchN_PORTALGUN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 5.55 -0.23 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin 10.49 -5.59 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1
	walkframe 25 LX LY
}

$animation "a_crouchNW_PORTALGUN" "chell_animations_anims\a_crouchNW_PORTALGUN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 0.23 5.09 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin 12.42 -0.78 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1
	walkframe 25 LX LY
}

$animation "a_crouchW_PORTALGUN" "chell_animations_anims\a_crouchW_PORTALGUN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 9.13 -2.66 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin -9.78 -5.29 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1
	walkframe 25 LX LY
}

$animation "a_crouchSW_PORTALGUN" "chell_animations_anims\a_crouchSW_PORTALGUN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 3.1 5.49 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin 9.33 -13.26 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1
	walkframe 25 LX LY
}

$animation "a_crouchCenter" "chell_animations_anims\a_crouchCenter.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 12.07 -0.38 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 0 fakeorigin -0.28 -5.51 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 0 0 0 0 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1
	walkframe 25 LX LY
}

$animation "a_crouchS" "chell_animations_anims\a_crouchS.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 7.83 -0.33 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin -0.55 -4.19 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1
	walkframe 25 LX LY
}

$animation "a_crouchSE" "chell_animations_anims\a_crouchSE.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 8.62 0.63 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin 5.57 -17.96 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1
	walkframe 25 LX LY
}

$animation "a_crouchE" "chell_animations_anims\a_crouchE.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 15.54 -1.94 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin 4.83 -14.71 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1
	walkframe 25 LX LY
}

$animation "a_crouchNE" "chell_animations_anims\a_crouchNE.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 9.99 0.11 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin 2.63 -6.22 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1
	walkframe 25 LX LY
}

$animation "a_crouchN" "chell_animations_anims\a_crouchN.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 5.55 -0.23 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin 10.49 -5.59 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1
	walkframe 25 LX LY
}

$animation "a_crouchNW" "chell_animations_anims\a_crouchNW.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 0.23 5.09 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin 12.42 -0.78 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1
	walkframe 25 LX LY
}

$animation "a_crouchW" "chell_animations_anims\a_crouchW.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 9.13 -2.66 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin -9.21 -5.6 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1
	walkframe 25 LX LY
}

$animation "a_crouchSW" "chell_animations_anims\a_crouchSW.smd" {
	fps 30
	loop
	ikrule "lfoot" footstep contact 1 fakeorigin 3.1 5.49 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 3
	ikrule "rfoot" footstep contact 1 fakeorigin 9.33 -13.26 0 fakerotate 0 0 0 floor 0 height 18 radius 0 range 14 25 2 14 target 2
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1
	walkframe 25 LX LY
}

$sequence "head_rot_z" {
	"a_head_rot_z_lt"
	"a_head_rot"
	"a_head_rot_z_rt"
	autoplay
	blend "head_yaw" -29.28032 29.24085
	blendwidth 3
	delta
	fadein 0.2
	fadeout 0.2
	hidden
}

$sequence "head_rot_y" {
	"a_head_rot_dwn"
	"a_head_rot"
	"a_head_rot_up"
	autoplay
	blend "head_pitch" 21.41426 -20.75119
	blendwidth 3
	delta
	fadein 0.2
	fadeout 0.2
	hidden
}

$sequence "portal_aimmatrix" {
	"a_portal_aimmatrix_up_right"
	"a_portal_aimmatrix_up_center"
	"a_portal_aimmatrix_up_left"
	"a_portal_aimmatrix_mid_right"
	"a_portal_aimmatrix_mid_center"
	"a_portal_aimmatrix_mid_left"
	"a_portal_aimmatrix_down_right"
	"a_portal_aimmatrix_down_center"
	"a_portal_aimmatrix_down_left"
	blend "body_yaw" 45 -45
	blend "body_pitch" -45 45
	delta
	fadein 0.2
	fadeout 0.2
	iklock "rfoot" 1 0
	iklock "lfoot" 1 0
}

$sequence "run_aim_matrix" {
	"a_run_aimmatrix_up_right"
	"a_run_aimmatrix_up_center"
	"a_run_aimmatrix_up_left"
	"a_run_aimmatrix_mid_right"
	"a_run_aimmatrix_mid_center"
	"a_run_aimmatrix_mid_left"
	"a_run_aimmatrix_down_right"
	"a_run_aimmatrix_down_center"
	"a_run_aimmatrix_down_left"
	blend "body_yaw" 45 -45
	blend "body_pitch" -45 45
	delta
	fadein 0.2
	fadeout 0.2
	iklock "rfoot" 1 0
	iklock "lfoot" 1 0
}

$sequence "portalgun_crouch_aim_matrix" {
	"a_crouch_aimmatrix_up_right"
	"a_crouch_aimmatrix_up_center"
	"a_crouch_aimmatrix_up_left"
	"a_crouch_aimmatrix_mid_right"
	"a_crouch_aimmatrix_mid_center"
	"a_crouch_aimmatrix_mid_left"
	"a_crouch_aimmatrix_down_right"
	"a_crouch_aimmatrix_down_center"
	"a_crouch_aimmatrix_down_left"
	blend "body_yaw" 45 -45
	blend "body_pitch" -45 45
	delta
	fadein 0.2
	fadeout 0.2
	iklock "rfoot" 1 0
	iklock "lfoot" 1 0
}

$sequence "stand_fire_PORTALGUN" {
	"a_stand_fire_PORTALGUN"
	activity "ACT_MP_ATTACK_STAND_PRIMARYFIRE" 1
	delta
	fadein 0.2
	fadeout 0.2
}

$sequence "crouch_fire_PORTALGUN" {
	"a_crouch_fire_PORTALGUN"
	activity "ACT_MP_ATTACK_CROUCH_PRIMARYFIRE" 1
	delta
	fadein 0.2
	fadeout 0.2
}

$sequence "idle_PORTALGUN" {
	"a_idle_PORTALGUN"
	activity "ACT_MP_STAND_PRIMARY" 1
	fadein 0.2
	fadeout 0.2
	addlayer "run_aim_matrix"
}

$sequence "Idle" {
	"a_idle"
	activity "ACT_MP_STAND_IDLE" 1
	fadein 0.2
	fadeout 0.2
}

$sequence "idleCROUCH_PORTALGUN" {
	"a_CROUCHidle_PORTALGUN"
	activity "ACT_MP_CROUCH_PRIMARY" 1
	fadein 0.2
	fadeout 0.2
	addlayer "portalgun_crouch_aim_matrix"
}

$sequence "CrouchIdle" {
	"a_CrouchIdle"
	activity "ACT_MP_CROUCH_IDLE" 1
	fadein 0.2
	fadeout 0.2
}

$sequence "Run_PORTALGUN" {
	"a_runS_PORTALGUN" "a_runSE_PORTALGUN" "a_runE_PORTALGUN" "a_runNE_PORTALGUN" "a_runN_PORTALGUN" "a_runNW_PORTALGUN" "a_runW_PORTALGUN" "a_runSW_PORTALGUN" "a_runS_PORTALGUN"
	activity "ACT_MP_RUN_PRIMARY" 1
	blendwidth 9
	blend move_yaw -180 180
	fadein 0.2
	fadeout 0.2
}

$sequence "Run" {
	"a_runS" "a_runSE" "a_runE" "a_runNE" "a_runN" "a_runNW" "a_runW" "a_runSW" "a_runS"
	activity "ACT_MP_RUN" 1
	blendwidth 9
	blend move_yaw -180 180
	fadein 0.2
	fadeout 0.2
}

$sequence "Crouch_walk_PORTALGUN" {
	"a_crouchS_PORTALGUN" "a_crouchSE_PORTALGUN" "a_crouchE_PORTALGUN" "a_crouchNE_PORTALGUN" "a_crouchN_PORTALGUN" "a_crouchNW_PORTALGUN" "a_crouchW_PORTALGUN" "a_crouchSW_PORTALGUN" "a_crouchS_PORTALGUN"
	activity "ACT_MP_CROUCHWALK_PRIMARY" 1
	blendwidth 9
	blend move_yaw -180 180
	fadein 0.2
	fadeout 0.2
}

$sequence "CrouchWalk" {
	"a_crouchS" "a_crouchSE" "a_crouchE" "a_crouchNE" "a_crouchN" "a_crouchNW" "a_crouchW" "a_crouchSW" "a_crouchS"
	activity "ACT_MP_CROUCHWALK" 1
	blendwidth 9
	blend move_yaw -180 180
	fadein 0.2
	fadeout 0.2
}

$sequence "portalgun_standing_jump" {
	"chell_animations_anims\portalgun_standing_jump.smd"
	activity "ACT_MP_JUMP_START_primary" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	ikrule "lfoot" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 3 usesequence 
	ikrule "rfoot" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 2 usesequence 
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0 usesequence 
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1 usesequence 
	walkframe 16
}

$sequence "standing_jump" {
	"chell_animations_anims\standing_jump.smd"
	activity "ACT_MP_JUMP_start" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	ikrule "lfoot" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 3 usesequence 
	ikrule "rfoot" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 2 usesequence 
	ikrule "rhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 0 usesequence 
	ikrule "lhand" release contact 0 fakeorigin 0 0 0 fakerotate 0 0 0 floor 0 height 0 radius 0 range 0 0 25 25 target 1 usesequence 
	walkframe 16
}

//needs to be included to fix player snapping 180 degrees when looking up and down, those animations don't actually work on Chell's, only portal gun ones work
$includemodel "player/female_anims.mdl"
