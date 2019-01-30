#include "ImUtil.h"
#include "TensorIter-inl.h"
#include "TensorApplyDim-inl.h"
#include "TensorApply-inl.h"
#include "math.h"

template<typename T>
INLINE_FUNC void im2cols(const int n, const TensorRef* data_im,
	const int height, const int width,
	const int ksize_h, const int ksize_w,
	const int pad_h, const int pad_w,
	const int stride_h, const int stride_w,
	const int dilation_h, const int dilation_w,
	const int height_col, const int width_col,
	TensorRef* data_col) {
	T* data_im_p = (T*)data_im->buffer;
	T* data_col_p = (T*)data_col->buffer;
	for (int index = 0; index < n; index++)
	{
		int w_out = index % width_col;
		index /= width_col;
		int h_out = index % height_col;
		int channel_in = index / height_col;
		int channel_out = channel_in * ksize_h * ksize_w;
		int h_in = h_out * stride_h - pad_h;
		int w_in = w_out * stride_w - pad_w;
		T *data_col_p_k = data_col_p + (channel_out * height_col + h_out) * width_col + w_out;
		T *data_im_p_k = data_im_p + (channel_in * height + h_in) * width + w_in;
		for (int i = 0; i < ksize_h; ++i) {
			for (int j = 0; j < ksize_w; ++j) {
				int h = h_in + i * dilation_h;
				int w = w_in + j * dilation_w;
				data_col_p_k = (float)(h >= 0 && w >= 0 && h < height && w < width) ?
					data_im_p_k[i * dilation_h * width + j * dilation_w] : 0;
				*data_col_p_k += height_col * width_col;
			}
		}
	}
}

template<typename T>
INLINE_FUNC void cols2im(const int n, float* data_col,
	const int height, const int width, const int channels,
	const int kernel_h, const int kernel_w,
	const int pad_h, const int pad_w,
	const int stride_h, const int stride_w,
	const int dilation_h, const int dilation_w,
	const int height_col, const int width_col,
	float* data_im) {
	for (int index = 0; index < n; index++)
	{
		float val = 0;
		const int w_im = index % width + pad_w;
		const int h_im = (index / width) % height + pad_h;
		const int c_im = index / (width * height);
		int kernel_extent_w = (kernel_w - 1) * dilation_w + 1;
		int kernel_extent_h = (kernel_h - 1) * dilation_h + 1;
		// compute the start and end of the output
		const int w_col_start =
			(w_im < kernel_extent_w) ? 0 : (w_im - kernel_extent_w) / stride_w + 1;
		const int w_col_end = (int)fmin(w_im / stride_w + 1, width_col);
		const int h_col_start =
			(h_im < kernel_extent_h) ? 0 : (h_im - kernel_extent_h) / stride_h + 1;
		const int h_col_end = (int)fmin(h_im / stride_h + 1, height_col);
		// TODO: use LCM of stride and dilation to avoid unnecessary loops
		for (int h_col = h_col_start; h_col < h_col_end; h_col += 1) {
			for (int w_col = w_col_start; w_col < w_col_end; w_col += 1) {
				int h_k = (h_im - h_col * stride_h);
				int w_k = (w_im - w_col * stride_w);
				if (h_k % dilation_h == 0 && w_k % dilation_w == 0) {
					h_k /= dilation_h;
					w_k /= dilation_w;
					int data_col_index = (((c_im * kernel_h + h_k) * kernel_w + w_k) *
						height_col + h_col) * width_col + w_col;
					val = val + data_col[data_col_index];
				}
			}
		}

		data_im[index] = val;
	}
}

int TS_Im2Cols(const int n, const TensorRef* data_im,
	const int height, const int width,
	const int ksize_h, const int ksize_w,
	const int pad_h, const int pad_w,
	const int stride_h, const int stride_w,
	const int dilation_h, const int dilation_w,
	const int height_col, const int width_col,
	TensorRef* data_col)
{
	API_BEGIN()
		im2cols<float>(n, data_im, height, width, ksize_h, ksize_w, pad_h, pad_w, stride_h, stride_w, dilation_h, dilation_w, height_col, width_col, data_col);
	API_END()
}

int TS_Cols2Im(const int n, float* data_col,
	const int height, const int width, const int channels,
	const int kernel_h, const int kernel_w,
	const int pad_h, const int pad_w,
	const int stride_h, const int stride_w,
	const int dilation_h, const int dilation_w,
	const int height_col, const int width_col,
	float* data_im)
{
	API_BEGIN()
		cols2im<float>(n, data_col, height, width, channels, kernel_h, kernel_w, pad_h, pad_w, stride_h, stride_w, dilation_h, dilation_w, height_col, width_col, data_im);
	API_END()
}