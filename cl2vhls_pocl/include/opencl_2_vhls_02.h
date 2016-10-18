/* File: opencl_2_vhls_02.h
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

#ifndef __OPENCL_2_VHLS_H__
#define __OPENCL_2_VHLS_H__

#include <iostream>
#include <fstream>

#include "llvm/Support/Host.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/IR/Module.h"

#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include <clang/Frontend/Utils.h>
#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/LangOptions.h"

#include "clang/Parse/Parser.h"
#include "clang/Parse/ParseAST.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Stmt.h"

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "clang/tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/Refactoring.h"


extern int globalOrderIndex;
extern int globalDimention;
extern std::string expressionsInKernel;
typedef enum portType {
	NOTUSED, INPUT, OUTPUT, INOUTPUT
};


void parse_expression(std::string expr);

class KernelPort {
public:
	KernelPort(std::string pnm, bool isP, bool isG, bool isL, bool isC) : portName(pnm), isPointer(isP), isGlobal(isG), isLocal(isL), isConst(isC), type(NOTUSED){}

	std::string portName;
	portType    type;
	bool        isPointer;
	bool        isGlobal;
	bool        isLocal;
	bool        isConst;
};




std::list<KernelPort>  ports;
const clang::ReturnStmt *return_stmt;

clang::SourceRange kernelSourceRange;


int setPortAsOutput( std::string varName) {
	for (std::list<KernelPort>::iterator index = ports.begin(), index_end = ports.end(); index != index_end; index++) {
		if (index->portName.compare(varName) == 0) {
			if (index->type == INPUT || index->type == INOUTPUT)
				index->type = INOUTPUT;
			else
				index->type = OUTPUT;
		}
	}
	return 0;
}


class IndexSpaceMatcherPrinter : public clang::ast_matchers::MatchFinder::MatchCallback {
public:

	IndexSpaceMatcherPrinter(clang::Rewriter &rew, clang::tooling::Replacements &rep) : theRewriter(rew), replace(rep), insideKernelFunction(false){}
	

	virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) {

		if (const clang::CallExpr *i_s_m = Result.Nodes.getNodeAs<clang::CallExpr>("indexSpace")) {
			std::cout << " value dump====start " << std::endl;
			i_s_m->getArg(0)->dump();
			std::cout << " value dump====end " << std::endl;
			if (const clang::ImplicitCastExpr *implisitCast = llvm::dyn_cast<clang::ImplicitCastExpr>(i_s_m->getArg(0))) {
				if (const clang::IntegerLiteral *intergerLiteral = llvm::dyn_cast<clang::IntegerLiteral>(implisitCast->getSubExpr())) {
					std::string indexString = intergerLiteral->getValue().toString(10, true);
					theRewriter.ReplaceText(i_s_m->getSourceRange(), "index_space_" + indexString);
					globalDimention++;
				}
			}
		}
		
		if (return_stmt = Result.Nodes.getNodeAs<clang::ReturnStmt>("returnStatement")) {
			std::string pragmas = "\n#pragma HLS INTERFACE ap_bus port=memory\n";
			pragmas += "#pragma HLS RESOURCE core=AXI4M variable=memory\n";
			pragmas += "#pragma HLS RESOURCE core=AXI4LiteS variable=return metadata=\" -bus_bundle LITE\" \n";
			theRewriter.RemoveText(return_stmt->getSourceRange());
			
			std::string forStatments;
			for (int i = 0; i < globalDimention; i++) {
				forStatments += "\n for(int index_space_" + std::to_string(i);
				forStatments += " = 0; index_space_" + std::to_string(i);
				forStatments += " < DIM_" + std::to_string(i) + "_MAX; index_space_" + std::to_string(i);
				forStatments += "++) {\n";
			}
			std::string closingBrackets;
			for (int i = 0; i < globalDimention; i++) {
				closingBrackets += "\n }\n";
			}
			std::string defineBuffer;
			std::string inputMemCopies;
			std::string outputMemCopies;

			for (std::list<KernelPort>::iterator index = ports.begin(), index_end = ports.end(); index != index_end; index++) {
				if (index->isPointer == true) {
					pragmas += "#pragma HLS INTERFACE ap_none register     port = " + index->portName + "_offset\n";
					pragmas += "#pragma HLS RESOURCE core=AXI4LiteS        variable=" + index->portName + "_offset" + " metadata=\" -bus_bundle LITE\"\n";
					defineBuffer += "float " + index->portName + "[MAX_SIZE];\n";
					if (index->type == INPUT || index->type == NOTUSED)
						inputMemCopies += "memcpy(" + index->portName + ",(float *)(memory+" + index->portName + "_offset), (n)*sizeof(float));\n";
					else
						outputMemCopies += "memcpy((float *)(memory+" + index->portName + "_offset)," + index->portName + ", (n)*sizeof(float));\n";
				}
				else {
					pragmas += "#pragma HLS INTERFACE ap_none register     port = " + index->portName + "\n";
					pragmas += "#pragma HLS RESOURCE core=AXI4LiteS        variable=" + index->portName + " metadata=\" -bus_bundle LITE\"\n";
				}

			}


			
			theRewriter.InsertTextAfterToken(kernelSourceRange.getBegin(), pragmas + defineBuffer + inputMemCopies + forStatments);
			theRewriter.InsertText(kernelSourceRange.getEnd(), closingBrackets+outputMemCopies);

			insideKernelFunction = false;
		}

		if (const clang::BinaryOperator *binary_stmt = Result.Nodes.getNodeAs<clang::BinaryOperator>("singleAssignement")) {
			//std::cout << "i from BinaryOperator matcher = " << globalOrderIndex++ << std::endl;

			clang::LangOptions LangOpts;
			LangOpts.OpenCL = true;
			clang::PrintingPolicy Policy(LangOpts);

			std::string TypeS;
			llvm::raw_string_ostream s(TypeS);
			binary_stmt->printPretty(s, 0, Policy);

			expressionsInKernel += "<index tyep=expression>";
			expressionsInKernel += s.str();
			expressionsInKernel += "</index>\n";


			clang::Expr *LHS=NULL;
			if (clang::ArraySubscriptExpr *Cast = llvm::dyn_cast<clang::ArraySubscriptExpr>(binary_stmt->getLHS())) {
				LHS = Cast->getLHS();
				if (clang::ImplicitCastExpr *Cast = llvm::dyn_cast<clang::ImplicitCastExpr>(LHS)) {
					LHS = Cast->getSubExpr();
				}
			}

			if (clang::ImplicitCastExpr *Cast = llvm::dyn_cast<clang::ImplicitCastExpr>(binary_stmt->getLHS())) {
				LHS = Cast->getSubExpr();
			}

			if (LHS != NULL) {
				if (const clang::DeclRefExpr *DeclRef = llvm::dyn_cast<clang::DeclRefExpr>(LHS)) {
					std::string varName = DeclRef->getNameInfo().getAsString();
					setPortAsOutput(varName);
				}
			}
		}


		if (const clang::VarDecl *vadDecExpr = Result.Nodes.getNodeAs<clang::VarDecl>("varDecl")) {


		}
		
		if (const clang::FunctionDecl *k_f_m = Result.Nodes.getNodeAs<clang::FunctionDecl>("kernelFunction")) {
			insideKernelFunction = true;
			//std::cout << "i from kernelFunction matcher = " << globalOrderIndex++ << std::endl;
			std::cout << "__KERNEL__" << std::endl;
			
			std::string defineBuffer;
			std::string memcopies;
			

			
			unsigned numberOfParams = k_f_m->getNumParams();
			for (int i = 0; i < numberOfParams; i++) {
				bool argPointer = false;
				bool argGlobal = false;
				bool argLocal = false;
				bool argConst = false;

				std::string argName = k_f_m->getParamDecl(i)->getDeclName().getAsString();

				std::string argType = k_f_m->getParamDecl(i)->getType().getAsString();
				if (argType.find("*") != std::string::npos) { // argument is a pointer
					argPointer = true;
					argType.replace(argType.find("*"), 1, "");
				}

				if (argType.find("__global") != std::string::npos) {
					argGlobal = true;
					argType.replace(argType.find("__global"), 8, "");
				}

				if (argType.find("__local") != std::string::npos) {
					argLocal = true;
					argType.replace(argType.find("__local"), 7, "");
				}

				if (argType.find("const") != std::string::npos) {
					argConst = true;
					argType.replace(argType.find("const"), 5, "");
				}


				std::string  replaceString;
				if (argPointer == true) {
					replaceString = " volatile u32 " + argName + "_offset" ;
				}
				else {
					replaceString = " volatile " + argType + " " + argName;
				}
				theRewriter.ReplaceText(k_f_m->getParamDecl(i)->getSourceRange(), replaceString);
				if (i == 0) {
					theRewriter.InsertTextBefore(k_f_m->getParamDecl(i)->getSourceRange().getBegin(), "volatile float* memory, \n");
				} 
				KernelPort p(argName, argPointer, argGlobal, argLocal, argConst);
				ports.push_back(p);
		
				
			}
			
		

			theRewriter.ReplaceText(k_f_m->getReturnTypeSourceRange(), "void");
			
			

			
			kernelSourceRange = k_f_m->getBody()->getSourceRange();

			
		}

		if (const clang::DeclRefExpr *d_r_e = Result.Nodes.getNodeAs<clang::DeclRefExpr>("varParamDecRef")) {
			
			/*
			clang::LangOptions LangOpts;
			LangOpts.OpenCL = true;
			clang::PrintingPolicy Policy(LangOpts);

			std::string TypeS;
			llvm::raw_string_ostream s(TypeS);
			d_r_e->printPretty(s, 0, Policy);

			expressionsInKernel += "<index tyep=expression>";
			expressionsInKernel += s.str();
			expressionsInKernel += "</index>\n";
			*/

			std::string varName = d_r_e->getNameInfo().getAsString();
			if (!varName.compare("a")) {
		//		theRewriter.ReplaceText(i_s_m->getSourceRange(), "a_offset");
			}
		}

		if (const clang::ArraySubscriptExpr *in_agr_access = Result.Nodes.getNodeAs<clang::ArraySubscriptExpr>("inputArrayAccess")) {

			if (const clang::ImplicitCastExpr *inVarExpr = llvm::dyn_cast<clang::ImplicitCastExpr>(in_agr_access->getLHS())) {
				if (const clang::DeclRefExpr *inVar = llvm::dyn_cast<clang::DeclRefExpr>(inVarExpr->getSubExpr())) {
					if (!inVar->getDecl()->getDeclName().getAsString().compare("imageIn")) {
						if (const clang::BinaryOperator *inVarIndex = llvm::dyn_cast<clang::BinaryOperator>(in_agr_access->getRHS())) {
							clang::LangOptions LangOpts;
							LangOpts.OpenCL = true;
							clang::PrintingPolicy Policy(LangOpts);

							std::string TypeS;
							llvm::raw_string_ostream s(TypeS);
							inVarIndex->printPretty(s, 0, Policy);

							expressionsInKernel += "<index tyep=input>";
							expressionsInKernel += s.str();
							//parse_expression(s.str());
							expressionsInKernel += "</index>\n";
						}
					}
				}
			}
		}
	}


private:

	clang::Rewriter                    &theRewriter;
	clang::tooling::Replacements       &replace;
	bool                                insideKernelFunction;
	
};

/*

class OpenCL2VHLSVisitor
	: public clang::RecursiveASTVisitor<OpenCL2VHLSVisitor> {
public:
	explicit OpenCL2VHLSVisitor(clang::ASTContext *Context, clang::Rewriter &R)
		: Context(Context), kernelCode(false), theRewriter(R), vhlsFileName("vhls.cpp"), skipFunctionArgument(false){
		vhlsFile.open(vhlsFileName);
	}

	bool VisitFunctionDecl(clang::FunctionDecl *funcDecl) {
		
		if (funcDecl->hasAttr<clang::OpenCLKernelAttr>()) {
			std::cout << "i from function declaration matcher = " << globalOrderIndex++ << std::endl;
			std::string funcName = funcDecl->getNameInfo().getName().getAsString();

			vhlsFile << "void    "; //vhls function return type
			vhlsFile << funcName;   //vhls function name
			vhlsFile << " (" << std::endl;
			
			vhlsFile << "  volatile float *memory" << std::endl;

			unsigned numberOfParams = funcDecl->getNumParams();
			for (int i = 0; i < numberOfParams; i++) {
				
				
				bool argPointer = false;
				bool argGlobal = false;
				bool argLocal = false;
				bool argConst = false;

				std::string argName = funcDecl->getParamDecl(i)->getDeclName().getAsString();

				std::string argType = funcDecl->getParamDecl(i)->getType().getAsString();
				if (argType.find("*") != std::string::npos) { // argument is a pointer
					argPointer = true;
					argType.replace(argType.find("*"), 1, "");
				}

				if (argType.find("__global") != std::string::npos) {
					argGlobal = true;
					argType.replace(argType.find("__global"), 8, "");
				}

				if (argType.find("__local") != std::string::npos) {
					argLocal = true;
					argType.replace(argType.find("__local"), 7, "");
				}

				if (argType.find("const") != std::string::npos) {
					argConst = true;
					argType.replace(argType.find("const"), 5, "");
				}

				
				
				std::string  replaceString;
				if (argPointer == true) {
					replaceString = " volatile u32 " + argName + "_offset" + "\n";
				}
				else {
					replaceString = " volatile " + argType + " " + argName + "\n";
				}
				theRewriter.ReplaceText(funcDecl->getParamDecl(i)->getSourceRange(), replaceString);
				if (i == 0) {
					theRewriter.InsertTextBefore(funcDecl->getParamDecl(i)->getSourceRange().getBegin(), "volatile float* memory, \n");
				}

				KernelPort p(argName, argPointer, argGlobal, argLocal, argConst);
				ports.push_back(p);
			}
			vhlsFile << ") {" << std::endl;



			kernelCode = true;
		}
			


		
		return true;
	}



	bool VisitVarDecl(clang::VarDecl* varDecl) {
		
		if (kernelCode) {
			std::cout << "i from var declaration matcher = " << globalOrderIndex++ << std::endl;
			std::string varName = varDecl->getName();
			if (varDecl->isFunctionOrMethodVarDecl()) {
				vhlsFile << varDecl->getType().getAsString();
				//std::string  errorInfo = "Error";
				vhlsFile << " " << varDecl->getNameAsString();

				if (varDecl->hasInit()) {
					vhlsFile << " = ";
				}
				else {
					vhlsFile << " ; ";
				}
				//llvm::raw_fd_ostream vhlsLlvmOutStream(vhlsFileName.c_str(), errorInfo, llvm::sys::fs::F_Append);
				//varDecl->print(llvm::errs());


			}


		}
		return true;
	}
	
	bool VisitCallExpr(clang::CallExpr* callExpr) {
		if (kernelCode) {
			std::cout << "i from call expression matcher = " << globalOrderIndex++ << std::endl;
		}
		return true;
	}
	bool VisitDeclRefExpr(clang::DeclRefExpr* decRef) {
		
		std::string functionCall = decRef->getNameInfo().getAsString();
		if (kernelCode) {
			std::cout << "i from call Reference expression matcher = " << globalOrderIndex++ << std::endl;
			clang::ValueDecl *Decl = decRef->getDecl();
			if (clang::FunctionDecl *Func = clang::dyn_cast<clang::FunctionDecl>(Decl)) {
				vhlsFile << functionCall << " Hello ";
			}
			
			
			
			
			if (!functionCall.compare("get_global_id")) {
				skipFunctionArgument = true;
			} 
			
			
			
		}
		return true;
	}
	bool VisitReturnStmt(clang::ReturnStmt *ReturnStatement) {
		if (kernelCode) {
			std::cout << "i from return visitor = " << globalOrderIndex++ << std::endl;
		}
		return true;
	}
	bool VisitStmt(clang::Stmt *s) {
		if (kernelCode) {
			std::cout << "i from Visit stmt matcher = " << globalOrderIndex++ << std::endl;
			if (clang::isa<clang::IfStmt>(s)) {
				
				clang::IfStmt *IfStatement = clang::cast<clang::IfStmt>(s);
				clang::Stmt *Then = IfStatement->getThen();

				theRewriter.InsertText(Then->getLocStart(),
					"// the 'if' part\n",
					true, true);

				clang::Stmt *Else = IfStatement->getElse();
				if (Else)
					theRewriter.InsertText(Else->getLocStart(),
					"// the 'else' part\n",
					true, true);

				
				
			}
			
			
		}
		return true;
	}

private:
	clang::ASTContext		*Context;
	bool					 kernelCode;
	clang::Rewriter			 &theRewriter;
	bool                     skipFunctionArgument;
	std::string              vhlsFileName;
	std::ofstream            vhlsFile;
};



class OpenCL2VHLSConsumer : public clang::ASTConsumer {
public:
	explicit OpenCL2VHLSConsumer(clang::ASTContext *Context, clang::Rewriter &R)
		: Visitor(Context, R) {}


	virtual void HandleTranslationUnit(clang::ASTContext &Context) {

		// Traversing the translation unit decl via a RecursiveASTVisitor
		// will visit all nodes in the AST.
		Visitor.TraverseDecl(Context.getTranslationUnitDecl());
	}
private:
	// A RecursiveASTVisitor implementation.
	OpenCL2VHLSVisitor Visitor;
};

*/
#endif //__OPENCL_2_VHLS_H__

