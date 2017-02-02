// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Consumer.h"

#include "clang/AST/ASTContext.h"

using namespace clang;

namespace UnrealCodeAnalyzer
{
	void FConsumer::HandleTranslationUnit(ASTContext& Context)
	{
		Visitor.TraverseDecl(Context.getTranslationUnitDecl());
	}

	FConsumer::FConsumer(ASTContext* Context, StringRef InFile) :
		Visitor(Context, InFile)
	{ }

}
