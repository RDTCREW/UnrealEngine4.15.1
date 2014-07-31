// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SScaleBox.generated.h"

UENUM()
namespace EStretchDirection
{
	enum Type
	{
		/** Will scale the content up or down */
		Both,
		/** Will only make the content smaller, will never scale it larger than the content's desired size. */
		DownOnly,
		/** Will only make the content larger, will never scale it smaller than the content's desired size. */
		UpOnly
	};
}

UENUM()
namespace EStretch
{
	enum Type
	{
		/** Does not scale the content. */
		None,
		/** Scales the content non-uniformly filling the entire space of the area */
		Fill,
		/** Scales the content uniformly (preserving aspect ratio) until it can no longer scale the content without clipping it. */
		ScaleToFit,
		/** Scales the content uniformly (preserving aspect ratio), until all sides meet or exceed the size of the area.  Will result in clipping longer sides. */
		ScaleToFill
	};
}

/**
 * Allows you to place content with a desired size and have it scale to meet the constraints placed on this box's alloted area.  If
 * you needed to have a background image scale to fill an area but not become distorted with different aspect ratios, or if you need
 * to auto fit some text to an area, this is the control for you.
 */
class UMG_API SScaleBox : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SScaleBox)
	: _Content()
	, _HAlign(HAlign_Center)
	, _VAlign(VAlign_Center)
	, _StretchDirection(EStretchDirection::Both)
	, _Stretch(EStretch::None)
	{}
		/** Slot for this designers content (optional) */
		SLATE_DEFAULT_SLOT(FArguments, Content)

		SLATE_ARGUMENT(EHorizontalAlignment, HAlign)

		SLATE_ARGUMENT(EVerticalAlignment, VAlign)
		
		/** Controls in what direction content can be scaled */
		SLATE_ATTRIBUTE(EStretchDirection::Type, StretchDirection)
		
		/** The stretching rule to apply when content is stretched */
		SLATE_ATTRIBUTE(EStretch::Type, Stretch)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	/** See Content slot */
	void SetContent(TSharedRef<SWidget> InContent);

	/** See HAlign argument */
	void SetHAlign(EHorizontalAlignment HAlign);

	/** See VAlign argument */
	void SetVAlign(EVerticalAlignment VAlign);

	/** See StretchDirection argument */
	void SetStretchDirection(EStretchDirection::Type InStretchDirection);

	/** See Stretch argument */
	void SetStretch(EStretch::Type InStretch);
	
private:
	TAttribute<EStretchDirection::Type> StretchDirection;
	TAttribute<EStretch::Type> Stretch;
};
