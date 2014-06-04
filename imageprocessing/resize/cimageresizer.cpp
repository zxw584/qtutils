#include "cimageresizer.h"
#include "cimageinterpolationkernel.h"
#include <math.h>
#include <assert.h>
#include <time.h>

inline QRgb applyKernel(const CImageInterpolationKernelBase<float>& kernel, const QImage& source, int x, int y)
{
	assert(x >= 0 && y >= 0);

	float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;

	if (source.depth() == 32)
	{
		for (int k = y, k_kernel = 0; k < y + kernel.size() && k < source.height(); ++k, ++ k_kernel)
		{
			const QRgb * line = (const QRgb*)source.constScanLine(k);
			for (int i = x, i_kernel = 0; i < x + kernel.size() && i < source.width(); ++i, ++i_kernel)
			{
				r += qRed(line[i]) * kernel.coeff(i_kernel, k_kernel);
				g += qGreen(line[i]) * kernel.coeff(i_kernel, k_kernel);
				b += qBlue(line[i]) * kernel.coeff(i_kernel, k_kernel);
				a += qAlpha(line[i]) * kernel.coeff(i_kernel, k_kernel);
			}
		}
	}
	else
	{
		for (int i = x, i_kernel = 0; i < x + kernel.size() && i < source.width(); ++i, ++i_kernel)
			for (int k = y, k_kernel = 0; k < y + kernel.size() && k < source.height(); ++k, ++ k_kernel)
			{
				const QColor pixel(source.pixel(i, k));
				r += pixel.red() * kernel.coeff(i_kernel, k_kernel);
				g += pixel.green() * kernel.coeff(i_kernel, k_kernel);
				b += pixel.blue() * kernel.coeff(i_kernel, k_kernel);
				a += pixel.alpha() * kernel.coeff(i_kernel, k_kernel);
			}
	}
	
	return qRgba((int)(r + 0.5f), (int)(g + 0.5f), (int)(b + 0.5f), (int)(a + 0.5f));
}

static QSize operator* (const QSize& origSize, int k)
{
	return QSize(origSize.width()*k, origSize.height()*k);
}


static float ratio(const QSize& size)
{
	return (float)size.width() / size.height();
}

CImageResizer::CImageResizer()
{
}

QImage CImageResizer::resize(const QImage& source, const QSize& targetSize, CImageResizer::ResizeMethod method, AspectRatio aspectRatio)
{
	switch(method)
	{
	case DefaultQimageFast:
		return source.scaled(targetSize, aspectRatio == KeepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::FastTransformation);
	case DefaultQimageSmooth:
		return source.scaled(targetSize, aspectRatio == KeepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	case Bicubic:
		return bicubicInterpolation(source, targetSize, aspectRatio);
	default:
		return QImage();
	}
}

QImage CImageResizer::bicubicInterpolation(const QImage& source, const QSize& targetSize, CImageResizer::AspectRatio aspectRatio)
{
	if (targetSize.width() >= source.width() || targetSize.height() >= source.height())
		return source.scaled(targetSize, aspectRatio == KeepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	const time_t start = clock();
	QSize actualTargetSize = targetSize;
	if (ratio(targetSize) != ratio(source.size()))
		actualTargetSize = source.size().scaled(targetSize, aspectRatio == KeepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio);

	const QSize upscaledSourceSize = source.width() % actualTargetSize.width() != 0 ? source.size().scaled(actualTargetSize * (source.width() / actualTargetSize.width() + 1), Qt::KeepAspectRatio) : source.size();

	QImage dest(actualTargetSize, source.format());
	QImage upscaledSource(upscaledSourceSize == source.size() ? source : source.scaled(upscaledSourceSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

	CBicubicKernel kernel(upscaledSource.width() / actualTargetSize.width(), 0.5f);
	if (upscaledSource.depth() == 32)
	{
		for (int y = 0; y < actualTargetSize.height(); ++y)
		{
			QRgb * destLine = (QRgb*)dest.scanLine(y);
			for (int x = 0; x < actualTargetSize.width(); ++x)
				destLine[x] = applyKernel(kernel, upscaledSource, x*kernel.size(), y*kernel.size());
		}
	}
	else
	{
		for (int x = 0; x < actualTargetSize.width(); ++x)
			for (int y = 0; y < actualTargetSize.height(); ++y)
				dest.setPixel(x, y, applyKernel(kernel, upscaledSource, x*kernel.size(), y*kernel.size()));
	}
	

	qDebug() << "Resizing" << upscaledSource.size() << "to" << actualTargetSize << "took" << (clock() - start) / (float)CLOCKS_PER_SEC * 1000.0f << "ms";

	return dest;
}