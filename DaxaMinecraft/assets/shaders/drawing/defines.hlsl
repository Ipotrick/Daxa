// Helper defines
#define ALBEDO_TEXTURE 0
#define ALBEDO_DEBUG_POS 1
#define ALBEDO_DEBUG_NRM 2
#define ALBEDO_DEBUG_DIST 4
#define ALBEDO_DEBUG_RANDOM 5

// Which information to show as the albedo
#define ALBEDO ALBEDO_TEXTURE
// Whether to disable everything else and draw just the complexity
#define VISUALIZE_STEP_COMPLEXITY 0
#define ENABLE_X16 1
#define ENABLE_X64 0
// Whether to cast shadow rays
#define ENABLE_SHADOWS 1
// Whether to visualize the position that the view ray intersects
#define SHOW_PICK_POS 1
#define SHOW_DEBUG_BLOCKS 0
// Whether to variate the sample-space coordinates based on time
#define JITTER_VIEW 0
// Number of samples per axis (so a value of 4 means 16 samples)
#define SUBSAMPLE_N 1

// Visualize x_n grid (n can be 1, 2, 4, 8, 16, 32, or 64)
#define VISUALIZE_SUBGRID 0

#define BLOCKEDIT_RADIUS 64

#define MAX_STEPS (BLOCK_NX + BLOCK_NY + BLOCK_NZ)
