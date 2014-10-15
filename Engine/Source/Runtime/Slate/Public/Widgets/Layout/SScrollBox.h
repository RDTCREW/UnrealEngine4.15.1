// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "InertialScrollManager.h"

/** SScrollBox can scroll through an arbitrary number of widgets. */
class SLATE_API SScrollBox : public SCompoundWidget
{
public:
	/** A Slot that provides layout options for the contents of a scrollable box. */
	class SLATE_API FSlot : public TSlotBase<FSlot>, public TSupportsContentPaddingMixin<FSlot>
	{
	public:
		FSlot()
			: TSlotBase<FSlot>()
			// Set both vertical and horizontal alignment to fill by default.  During layout, the
			// alignment direction parallel to the scroll direction is assumed to be top, or left
			// since that is how the items are stacked.
			, HAlignment(HAlign_Fill)
			, VAlignment(VAlign_Fill)
		{
		}

		FSlot& HAlign( EHorizontalAlignment InHAlignment )
		{
			HAlignment = InHAlignment;
			return *this;
		}

		FSlot& VAlign(EVerticalAlignment InVAlignment)
		{
			VAlignment = InVAlignment;
			return *this;
		}
		
		EHorizontalAlignment HAlignment;
		EVerticalAlignment VAlignment;
	};

	SLATE_BEGIN_ARGS(SScrollBox)
		: _Style( &FCoreStyle::Get().GetWidgetStyle<FScrollBoxStyle>("ScrollBox") )
		, _ScrollBarStyle( &FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar") )
		, _ExternalScrollbar()
		, _Orientation(Orient_Vertical)
		, _OnUserScrolled()
		{}
		
		SLATE_SUPPORTS_SLOT( FSlot )

		/** Style used to draw this scrollbox */
		SLATE_STYLE_ARGUMENT( FScrollBoxStyle, Style )

		/** Style used to draw this scrollbox's scrollbar */
		SLATE_STYLE_ARGUMENT( FScrollBarStyle, ScrollBarStyle )

		/** Custom scroll bar */
		SLATE_ARGUMENT( TSharedPtr<SScrollBar>, ExternalScrollbar )

		/** The direction that children will be stacked, and also the direction the box will scroll. */
		SLATE_ARGUMENT(EOrientation, Orientation)

		/** Called when the button is clicked */
		SLATE_EVENT(FOnUserScrolled, OnUserScrolled)

	SLATE_END_ARGS()


	/** @return a new slot. Slots contain children for SScrollBox */
	static FSlot& Slot();

	void Construct( const FArguments& InArgs );

	/** Adds a slot to SScrollBox */
	SScrollBox::FSlot& AddSlot();

	/** Removes a slot at the specified location */
	void RemoveSlot( const TSharedRef<SWidget>& WidgetToRemove );

	/** Removes all children from the box */
	void ClearChildren();

	/** @return Returns true if the user is currently interactively scrolling the view by holding
		        the right mouse button and dragging. */
	bool IsRightClickScrolling() const;

	float GetScrollOffset();

	void SetScrollOffset( float NewScrollOffset );

	/** Get the current orientation of the scrollbox. */
	EOrientation GetOrientation();

	/** Sets the current orientation of the scrollbox and updates the layout */
	void SetOrientation(EOrientation InOrientation);

public:

	// SWidget interface
	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;
	virtual FReply OnMouseButtonDown( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	virtual FReply OnMouseButtonUp( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	virtual FReply OnMouseMove( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	virtual void OnMouseLeave( const FPointerEvent& MouseEvent ) override;
	virtual FReply OnMouseWheel( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	virtual FReply OnDragDetected( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	virtual FCursorReply OnCursorQuery( const FGeometry& MyGeometry, const FPointerEvent& CursorEvent ) const override;
	virtual int32 OnPaint( const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const override;
	// End of SWidget interface

private:

	/** Builds a default Scrollbar */
	TSharedPtr<SScrollBar> ConstructScrollBar();

	/** Constructs internal layout widgets for scrolling vertically using the existing ScrollPanel and ScrollBar. */
	void ConstructVerticalLayout();

	/** Constructs internal layout widgets for scrolling horizontally using the existing ScrollPanel and ScrollBar. */
	void ConstructHorizontalLayout();

	/** Gets the component of a vector in the direction of scrolling based on the Orientation property. */
	inline float GetScrollComponentFromVector(FVector2D Vector) const
	{
		return Orientation == Orient_Vertical ? Vector.Y : Vector.X;
	}

	/** Sets the component of a vector in the direction of scrolling based on the Orientation property. */
	inline void SetScrollComponentOnVector(FVector2D& InVector, float Value) const
	{
		if (Orientation == Orient_Vertical)
		{
			InVector.Y = Value;
		}
		else
		{
			InVector.X = Value;
		}
	}

	/** Scroll offset that the user asked for. We will clamp it before actually scrolling there. */
	float DesiredScrollOffset;

	/**
	 * Scroll the view by ScrollAmount given its currently AllottedGeometry.
	 *
	 * @param AllottedGeometry  The geometry allotted for this SScrollBox by the parent
	 * @param ScrollAmount      
	 * @return Whether or not the scroll was fully handled
	 */
	bool ScrollBy( const FGeometry& AllottedGeometry, float ScrollAmount, bool InAnimateScroll = true );

	/** Invoked when the user scroll via the scrollbar */
	void ScrollBar_OnUserScrolled( float InScrollOffsetFraction );

	/** Does the user need a hint that they can scroll to the start of the list? */
	FSlateColor GetStartShadowOpacity() const;
	
	/** Does the user need a hint that they can scroll to the end of the list? */
	FSlateColor GetEndShadowOpacity() const;

	/** The panel which stacks the child slots */
	TSharedPtr<class SScrollPanel> ScrollPanel;

	/** The scrollbar which controls scrolling for the scrollbox. */
	TSharedPtr<SScrollBar> ScrollBar;

	/** Whether or not the user supplied an external scrollbar to control scrolling. */
	bool bScrollBarIsExternal;
	
	/** Are we actively scrolling right now */
	bool bIsScrolling;

	/** Should the current scrolling be animated or immediately jump to the desired scroll offer */
	bool bAnimateScroll;

	/** How much we scrolled while the rmb has been held */
	float AmountScrolledWhileRightMouseDown;

	/** Helper object to manage inertial scrolling */
	FInertialScrollManager InertialScrollManager;

	/**	The current position of the software cursor */
	FVector2D SoftwareCursorPosition;

	/**	Whether the software cursor should be drawn in the viewport */
	bool bShowSoftwareCursor;

	/** Fired when the user scrolls the scrollbox */
	FOnUserScrolled OnUserScrolled;

	/** The scrolling and stacking orientation. */
	EOrientation Orientation;

	/** Style resource for the scrollbox */
	const FScrollBoxStyle* Style;

	/** Style resource for the scrollbar */
	const FScrollBarStyle* ScrollBarStyle;
};


