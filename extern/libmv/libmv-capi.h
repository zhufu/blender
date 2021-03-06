/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2011 Blender Foundation.
 * All rights reserved.
 *
 * Contributor(s): Blender Foundation,
 *                 Sergey Sharybin
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#ifndef LIBMV_C_API_H
#define LIBMV_C_API_H

#ifdef __cplusplus
extern "C" {
#endif

struct libmv_Tracks;
struct libmv_Reconstruction;
struct libmv_Features;
struct libmv_CameraIntrinsics;

/* Logging */
void libmv_initLogging(const char *argv0);
void libmv_startDebugLogging(void);
void libmv_setLoggingVerbosity(int verbosity);

/* TrackRegion (new planar tracker) */
struct libmv_trackRegionOptions {
	int motion_model;
	int num_iterations;
	int use_brute;
	int use_normalization;
	double minimum_correlation;
	double sigma;
	float *image1_mask;
};

struct libmv_trackRegionResult {
	int termination;
	const char *termination_reason;
	double correlation;
};

int libmv_trackRegion(const struct libmv_trackRegionOptions *options,
                      const float *image1, int image1_width, int image1_height,
                      const float *image2, int image2_width, int image2_height,
                      const double *x1, const double *y1,
                      struct libmv_trackRegionResult *result,
                      double *x2, double *y2);

void libmv_samplePlanarPatch(const float *image, int width, int height,
                             int channels, const double *xs, const double *ys,
                             int num_samples_x, int num_samples_y,
                             const float *mask, float *patch,
                             double *warped_position_x, double *warped_position_y);

/* Tracks */
struct libmv_Tracks *libmv_tracksNew(void);
void libmv_tracksInsert(struct libmv_Tracks *libmv_tracks, int image, int track, double x, double y);
void libmv_tracksDestroy(struct libmv_Tracks *libmv_tracks);

/* Reconstruction solver */

#define LIBMV_REFINE_FOCAL_LENGTH          (1 << 0)
#define LIBMV_REFINE_PRINCIPAL_POINT       (1 << 1)
#define LIBMV_REFINE_RADIAL_DISTORTION_K1  (1 << 2)
#define LIBMV_REFINE_RADIAL_DISTORTION_K2  (1 << 4)

typedef struct libmv_cameraIntrinsicsOptions {
	double focal_length;
	double principal_point_x, principal_point_y;
	double k1, k2, k3;
	double p1, p2;
	int image_width, image_height;
} libmv_cameraIntrinsicsOptions;

typedef struct libmv_reconstructionOptions {
	int select_keyframes;
	int keyframe1, keyframe2;

	int refine_intrinsics;

	double success_threshold;
	int use_fallback_reconstruction;
} libmv_reconstructionOptions;

typedef void (*reconstruct_progress_update_cb) (void *customdata, double progress, const char *message);

struct libmv_Reconstruction *libmv_solveReconstruction(const struct libmv_Tracks *libmv_tracks,
			const libmv_cameraIntrinsicsOptions *libmv_camera_intrinsics_options,
			libmv_reconstructionOptions *libmv_reconstruction_options,
			reconstruct_progress_update_cb progress_update_callback,
			void *callback_customdata);
struct libmv_Reconstruction *libmv_solveModal(const struct libmv_Tracks *libmv_tracks,
			const libmv_cameraIntrinsicsOptions *libmv_camera_intrinsics_options,
			const libmv_reconstructionOptions *libmv_reconstruction_options,
			reconstruct_progress_update_cb progress_update_callback,
			void *callback_customdata);
int libmv_reporojectionPointForTrack(const struct libmv_Reconstruction *libmv_reconstruction, int track, double pos[3]);
double libmv_reporojectionErrorForTrack(const struct libmv_Reconstruction *libmv_reconstruction, int track);
double libmv_reporojectionErrorForImage(const struct libmv_Reconstruction *libmv_reconstruction, int image);
int libmv_reporojectionCameraForImage(const struct libmv_Reconstruction *libmv_reconstruction, int image, double mat[4][4]);
double libmv_reprojectionError(const struct libmv_Reconstruction *libmv_reconstruction);
void libmv_destroyReconstruction(struct libmv_Reconstruction *libmv_reconstruction);

/* feature detector */
struct libmv_Features *libmv_detectFeaturesFAST(const unsigned char *data, int width, int height, int stride,
			int margin, int min_trackness, int min_distance);
struct libmv_Features *libmv_detectFeaturesMORAVEC(const unsigned char *data, int width, int height, int stride,
			int margin, int count, int min_distance);
int libmv_countFeatures(const struct libmv_Features *libmv_features);
void libmv_getFeature(const struct libmv_Features *libmv_features, int number, double *x, double *y, double *score, double *size);
void libmv_destroyFeatures(struct libmv_Features *libmv_features);

/* camera intrinsics */
struct libmv_CameraIntrinsics *libmv_ReconstructionExtractIntrinsics(struct libmv_Reconstruction *libmv_Reconstruction);

struct libmv_CameraIntrinsics *libmv_CameraIntrinsicsNewEmpty(void);

struct libmv_CameraIntrinsics *libmv_CameraIntrinsicsNew(const libmv_cameraIntrinsicsOptions *libmv_camera_intrinsics_options);

struct libmv_CameraIntrinsics *libmv_CameraIntrinsicsCopy(const struct libmv_CameraIntrinsics *libmv_intrinsics);

void libmv_CameraIntrinsicsDestroy(struct libmv_CameraIntrinsics *libmv_intrinsics);

void libmv_CameraIntrinsicsUpdate(const libmv_cameraIntrinsicsOptions *libmv_camera_intrinsics_options,
                                  struct libmv_CameraIntrinsics *libmv_intrinsics);

void libmv_CameraIntrinsicsSetThreads(struct libmv_CameraIntrinsics *libmv_intrinsics, int threads);

void libmv_CameraIntrinsicsExtract(const struct libmv_CameraIntrinsics *libmv_intrinsics, double *focal_length,
			double *principal_x, double *principal_y, double *k1, double *k2, double *k3, int *width, int *height);

void libmv_CameraIntrinsicsUndistortByte(const struct libmv_CameraIntrinsics *libmv_intrinsics,
			unsigned char *src, unsigned char *dst, int width, int height, float overscan, int channels);

void libmv_CameraIntrinsicsUndistortFloat(const struct libmv_CameraIntrinsics *libmv_intrinsics,
			float *src, float *dst, int width, int height, float overscan, int channels);

void libmv_CameraIntrinsicsDistortByte(const struct libmv_CameraIntrinsics *libmv_intrinsics,
			unsigned char *src, unsigned char *dst, int width, int height, float overscan, int channels);

void libmv_CameraIntrinsicsDistortFloat(const struct libmv_CameraIntrinsics *libmv_intrinsics,
			float *src, float *dst, int width, int height, float overscan, int channels);

/* utils */
void libmv_ApplyCameraIntrinsics(const libmv_cameraIntrinsicsOptions *libmv_camera_intrinsics_options,
			double x, double y, double *x1, double *y1);
void libmv_InvertCameraIntrinsics(const libmv_cameraIntrinsicsOptions *libmv_camera_intrinsics_options,
			double x, double y, double *x1, double *y1);

#ifdef __cplusplus
}
#endif

#endif // LIBMV_C_API_H
