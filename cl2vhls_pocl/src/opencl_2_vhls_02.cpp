/* File: opencl_2_vhls_02.cpp
 *
 Copyright (c) [2016] [Mohammad Hosseinabady (mohammad@hosseinabady.com)]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
===============================================================================
* This file has been written at University of Bristol
* for the ENPOWER project funded by EPSRC
*
* File name : matrix_mult.cpp
* author    : Mohammad hosseinabady mohammad@hosseinabady.com
* date      : 18 October 2016
*/

#include "opencl_2_vhls_02.h"

int iransform_expression(void);


int OpenCL_to_CPP(char* fileName, char* kernelhPath);

int	globalOrderIndex = 0;
int globalDimention=0;
std::string expressionsInKernel;

clang::ast_matchers::DeclarationMatcher   kernelDeclMatcher = clang::ast_matchers::functionDecl(clang::ast_matchers::hasName("image_filtering")).bind("kernelFunction");

clang::ast_matchers::StatementMatcher     returnStmtMatcher = clang::ast_matchers::returnStmt().bind("returnStatement");

clang::ast_matchers::StatementMatcher     indexSpaceMatcher = clang::ast_matchers::callExpr(clang::ast_matchers::callee(
	                                                        clang::ast_matchers::functionDecl(clang::ast_matchers::hasName(
															"get_global_id")))).bind("indexSpace");

clang::ast_matchers::StatementMatcher     inputArrayAccess = clang::ast_matchers::arraySubscriptExpr().bind("inputArrayAccess");

clang::ast_matchers::StatementMatcher   singleAssignmentOperatorMatcher = clang::ast_matchers::binaryOperator(clang::ast_matchers::hasOperatorName("=")).bind("singleAssignement");


clang::ast_matchers::StatementMatcher   declRefMatcher = clang::ast_matchers::declRefExpr().bind("varParamDecRef");

clang::ast_matchers::DeclarationMatcher   varDeclMatcher = clang::ast_matchers::varDecl().bind("varDecl");

int main(int argc, char *argv[]) {
	char  fileName[100];
	char  kernelhPath[100];
	if (argc > 2 && argc < 4) {
		strcpy(fileName, argv[1]);
		strcpy(kernelhPath, argv[2]);
	} else {
		printf("Please enter filename without extension\n");
		return -1;
	}
	OpenCL_to_CPP(fileName, kernelhPath);
}

int OpenCL_to_CPP(char* fileName, char* kernelhPath) {

	std::cout << "=====================================" << std::endl;
	std::cout << "           Hello OpenCL Clang!       " << std::endl;
	std::cout << "=====================================" << std::endl;
	std::cout << std::endl << std::endl;

	
	char  clFileName[100];
	char  bcFileName[100];
	char  cppFileName[100];
	//pars argument
	
	strcpy(clFileName, fileName);
	strcat(clFileName, ".cl");

	strcpy(bcFileName, fileName);
	strcat(bcFileName, ".bc");

	strcpy(cppFileName, fileName);
	strcat(cppFileName, ".cpp");

	
	//1 -- compilerInstance instantiation
	  // CI holds the Clang compiler instance for the rest of the code
	  // CI manages different other objects required in the compilation process
	clang::CompilerInstance CI;

	std::cout << __LINE__ << std::endl;

	//1.1 -- create diagnostics
	CI.createDiagnostics();
	//ToDo configure diagnostics here
	std::cout << __LINE__ << std::endl;

	//get invocation object which has been created by CompilerInstance
	//invocation object holds the data necessary to invoke 
	//the compiler, including paths, code generation options, warning flags
	clang::CompilerInvocation &invocation = CI.getInvocation();

	//1.2 -- provide language options
	//ToDo define the input language and its configuration here
	clang::LangOptions *la = invocation.getLangOpts();
	invocation.setLangDefaults(*la, clang::IK_OpenCL, clang::LangStandard::lang_opencl12);

	la->CharIsSigned = true;
	std::cout << __LINE__ << std::endl;

	la->OpenCLVersion = 120;
	la->FakeAddressSpaceMap = true;
	la->Blocks = true; //-fblocks
	la->MathErrno = false; // -fno-math-errno
	la->NoBuiltin = true;  // -fno-builtin

	//1.3 -- provide target info
	std::shared_ptr<clang::TargetOptions> ta = invocation.TargetOpts;
	ta->Triple = llvm::sys::getDefaultTargetTriple();

	clang::TargetInfo *pti = clang::TargetInfo::CreateTargetInfo(CI.getDiagnostics(), ta);
	CI.setTarget(pti);
	CI.getTarget().adjust(CI.getLangOpts());
	std::cout << __LINE__ << std::endl;

	//1.4 -- create source manager
	CI.createFileManager();
	CI.createSourceManager(CI.getFileManager());

	std::cout << __LINE__ << std::endl;
	const clang::FileEntry *pFile = CI.getFileManager().getFile(clFileName);
	llvm::MemoryBuffer *memoryFile = CI.getFileManager().getBufferForFile(pFile);
	CI.getSourceManager().setMainFileID(CI.getSourceManager().createFileID(memoryFile));
	

	
	std::cout << __LINE__ << std::endl;
	clang::FrontendOptions &fe = invocation.getFrontendOpts();
	fe.Inputs.clear();
	fe.Inputs.push_back(clang::FrontendInputFile(clFileName, clang::IK_OpenCL));
	fe.OutputFile = std::string(bcFileName);

	std::cout << __LINE__ << std::endl;

	//1.5 -- Header search
	//ToDo define and configure the head search object here
	clang::PreprocessorOptions &po = invocation.getPreprocessorOpts();
	std::cout << __LINE__ << std::endl;
	po.addMacroDef("__OPENCL_VERSION__=120"); // -D__OPENCL_VERSION_=120

	std::string kernelh;
	kernelh = kernelhPath;
	kernelh += "_kernel.h";
	po.Includes.push_back(kernelh);
	std::cout << __LINE__ << std::endl;
	//1.6 -- Module loader
	//ToDo configure the module loader object here


	clang::tooling::Replacements replacement;

	std::cout << __LINE__ << std::endl;
	clang::Rewriter theRewriter;
	theRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());

	CI.createPreprocessor(clang::TU_Complete);
	CI.createASTContext();
	std::cout << __LINE__ << std::endl;
	/*
	clang::ASTContext &cntx = CI.getASTContext();
	OpenCL2VHLSConsumer  *astConsumer = new OpenCL2VHLSConsumer(&cntx, theRewriter);

	CI.setASTConsumer(astConsumer);

	
	CI.createASTContext();
	CI.createSema(clang::TU_Complete, NULL);


	CI.getDiagnosticClient().BeginSourceFile(CI.getLangOpts(), &CI.getPreprocessor());
	clang::ParseAST(CI.getSema());
	CI.getDiagnosticClient().EndSourceFile();


*/

	std::cout << __LINE__ << std::endl;
	IndexSpaceMatcherPrinter printer(theRewriter, replacement);
	clang::ast_matchers::MatchFinder  finder;
	finder.addMatcher(indexSpaceMatcher, &printer);
	finder.addMatcher(kernelDeclMatcher, &printer);
	finder.addMatcher(declRefMatcher, &printer);
	finder.addMatcher(returnStmtMatcher, &printer);
	finder.addMatcher(singleAssignmentOperatorMatcher, &printer);
	finder.addMatcher(inputArrayAccess, &printer);
	finder.addMatcher(varDeclMatcher, &printer);
	

	std::unique_ptr<clang::ASTConsumer> pAstConsumer(finder.newASTConsumer());
	clang::DiagnosticConsumer& diagConsumer = CI.getDiagnosticClient();
	diagConsumer.BeginSourceFile(CI.getLangOpts(), &CI.getPreprocessor());

	std::cout << __LINE__ << std::endl;
	clang::ParseAST(CI.getPreprocessor(), pAstConsumer.get(), CI.getASTContext());
	//clang::ParseAST(CI.getPreprocessor(), &CI.getASTConsumer(), CI.getASTContext());
	
	diagConsumer.EndSourceFile();

	std::cout << __LINE__ << std::endl;
	std::cout << std::endl << std::endl;
	std::cout << "=====================================" << std::endl;
	std::cout << "           Bye OpenCL Clang!         " << std::endl;
	std::cout << "=====================================" << std::endl;


	const clang::RewriteBuffer *RewriteBuf =
		theRewriter.getRewriteBufferFor(CI.getSourceManager().getMainFileID());
//	llvm::outs() << std::string(RewriteBuf->begin(), RewriteBuf->end());
	std::string              vhlsFileName(cppFileName);
	std::string              variableExpressionFileName("variableExpression.xml");
	std::ofstream            vhlsFile(vhlsFileName);
	std::ofstream            variableExpressionFile(variableExpressionFileName);

	std::cout << __LINE__ << std::endl;
	vhlsFile << std::string(RewriteBuf->begin(), RewriteBuf->end());

	variableExpressionFile << expressionsInKernel << std::endl;
	
	iransform_expression();
	return 0;
}