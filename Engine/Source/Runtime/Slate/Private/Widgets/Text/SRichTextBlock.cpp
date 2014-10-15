// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "SlatePrivatePCH.h"

#if WITH_FANCY_TEXT

DECLARE_CYCLE_STAT( TEXT("OnPaint SRichTextBlock"), STAT_SlateOnPaint_SRichTextBlock, STATGROUP_Slate );

#include "TextLayoutEngine.h"
#include "TextBlockLayout.h"
#include "IRichTextMarkupParser.h"
#include "RichTextMarkupProcessing.h"
#include "RichTextLayoutMarshaller.h"

void SRichTextBlock::Construct( const FArguments& InArgs )
{
	BoundText = InArgs._Text;
	HighlightText = InArgs._HighlightText;

	TextStyle = *InArgs._TextStyle;
	WrapTextAt = InArgs._WrapTextAt;
	AutoWrapText = InArgs._AutoWrapText;
	Margin = InArgs._Margin;
	LineHeightPercentage = InArgs._LineHeightPercentage;
	Justification = InArgs._Justification;

	{
		TSharedPtr<IRichTextMarkupParser> Parser = InArgs._Parser;
		if ( !Parser.IsValid() )
		{
			Parser = FDefaultRichTextMarkupParser::Create();
		}

		TSharedRef<FRichTextLayoutMarshaller> Marshaller = FRichTextLayoutMarshaller::Create(Parser, nullptr, InArgs._Decorators, InArgs._DecoratorStyleSet);
		for ( const TSharedRef< ITextDecorator >& Decorator : InArgs.InlineDecorators )
		{
			Marshaller->AppendInlineDecorator( Decorator );
		}

		TextLayoutCache = FTextBlockLayout::Create(TextStyle, Marshaller, nullptr);
	}
}

int32 SRichTextBlock::OnPaint( const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const
{
	SCOPE_CYCLE_COUNTER( STAT_SlateOnPaint_SRichTextBlock );

	// OnPaint will also update the text layout cache if required
	LayerId = TextLayoutCache->OnPaint(
		FTextBlockLayout::FWidgetArgs(BoundText, HighlightText, WrapTextAt, AutoWrapText, Margin, LineHeightPercentage, Justification), 
		Args.WithNewParent(this), AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, ShouldBeEnabled(bParentEnabled)
		);

	return LayerId;
}

FVector2D SRichTextBlock::ComputeDesiredSize() const
{
	// todo: jdale - The scale needs to be passed to ComputeDesiredSize
	//const float Scale = CachedScale;

	// ComputeDesiredSize will also update the text layout cache if required
	const FVector2D TextSize = TextLayoutCache->ComputeDesiredSize(
		FTextBlockLayout::FWidgetArgs(BoundText, HighlightText, WrapTextAt, AutoWrapText, Margin, LineHeightPercentage, Justification), 
		/*Scale, */TextStyle
		);

	return TextSize;
}

FChildren* SRichTextBlock::GetChildren()
{
	return TextLayoutCache->GetChildren();
}

void SRichTextBlock::OnArrangeChildren( const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren ) const
{
	TextLayoutCache->ArrangeChildren( AllottedGeometry, ArrangedChildren );
}

void SRichTextBlock::SetText( const TAttribute<FText>& InTextAttr )
{
	BoundText = InTextAttr;
}

void SRichTextBlock::SetHighlightText( const TAttribute<FText>& InHighlightText )
{
	HighlightText = InHighlightText;
}

void SRichTextBlock::Refresh()
{
	TextLayoutCache->DirtyLayout();
}

#endif //WITH_FANCY_TEXT
