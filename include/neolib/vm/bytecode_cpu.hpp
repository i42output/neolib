// bytecode_cpu.hpp
/*
 *  Copyright (c) 2007 Leigh Johnston.
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *     * Neither the name of Leigh Johnston nor the names of any
 *       other contributors to this software may be used to endorse or
 *       promote products derived from this software without specific prior
 *       written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <neolib/neolib.hpp>
#include "i_cpu.hpp"

namespace neolib
{
	namespace vm
	{
		class bytecode_cpu : public i_cpu
		{
		public:
			class opcode
			{
			public:
				enum : uint8_t
				{
					Nop			= 0x00,
					Load		= 0x01,
					Store		= 0x02,
					Push		= 0x03,
					Pop			= 0x04,

					Jmp			= 0x10,
					Call		= 0x11,
					Br			= 0x12,
					Cmp			= 0x13,
					Cmpxchg		= 0x14,
					BrEq		= 0x15,
					BrNEq		= 0x16,
					BrGt		= 0x17,
					BrLt		= 0x18,
					BrGtEq		= 0x19,
					BrLtEq		= 0x1A,

					Add			= 0x20,
					Sub			= 0x21,
					UMul		= 0x22,
					UDiv		= 0x23,
					SMul		= 0x24,
					SDiv		= 0x25,
					Shl			= 0x26,
					Shr			= 0x27,
					Neg			= 0x28,
					And			= 0x29,
					Or			= 0x30,
					Xor			= 0x31,

					Int			= 0xF0,
					Exec		= 0xF1,
				};
			};
			class argument
			{
				enum : uint8_t
				{
					Integer8	= 0x00,
					Integer16	= 0x01,
					Integer32	= 0x02,
					Integer64	= 0x03,
					Float32		= 0x04,
					Float64		= 0x05,
					R0			= 0x10,
					R1			= 0x11,
					R2			= 0x12,
					R3			= 0x13,
					R4			= 0x14,
					R5			= 0x15,
					R6			= 0x16,
					R7			= 0x17,
					FPR0		= 0x20,
					FPR1		= 0x21,
					FPR2		= 0x22,
					FPR3		= 0x23,
					FPR4		= 0x24,
					FPR5		= 0x25,
					FPR6		= 0x26,
					FPR7		= 0x27
				};
			};
			struct context
			{
				page* stack;
				uint32_t flags;
				const uint8_t* IP;
				uint8_t* SP;
				int64_t R[8];
				double FPR[8];
			};
			template <std::size_t N>
			class arguments
			{
				struct ref
				{
					uint8_t* ptr;
					uint8_t len;
				};
			public:
				arguments(context& aContext)
				{
				}
			public:
				std::array<ref, N> arg;
			};
		public:
			virtual uint32_t cores() const
			{
				/* todo */
				return 1;
			}
			virtual uint32_t threads() const
			{
				/* todo */
				return 1;
			}
		public:
			virtual page* allocate_text_page(uint32_t aSize = 64 * 1024)
			{
				iText.emplace_back(aSize);
				return &iText.back();
			}
			virtual page* allocate_stack_page(uint32_t aSize = 1024 * 1024)
			{
				iStacks.emplace_back(aSize);
				return &iStacks.back();
			}
		public:
			virtual void execute(const uint8_t* aEntryPoint)
			{
				context newContext{ allocate_stack_page() };
				newContext.IP = aEntryPoint;
				newContext.SP = &newContext.stack->front() + newContext.stack->size();
				process(newContext);
			}
		private:
			void process(context& aContext)
			{
				switch (*aContext.IP++)
				{
				case opcode::Nop:
					break;
				case opcode::Load:
					{
						arguments<2> args{ aContext };
					}
					break;
				case opcode::Store:
					{
						arguments<2> args{ aContext };
					}
					break;
				case opcode::Push:
					break;
				case opcode::Pop:
					break;
				case opcode::Jmp:
					break;
				case opcode::Call:
					break;
				case opcode::Br:
					break;
				case opcode::Cmp:
					break;
				case opcode::Cmpxchg:
					break;
				case opcode::BrEq:
					break;
				case opcode::BrNEq:
					break;
				case opcode::BrGt:
					break;
				case opcode::BrLt:
					break;
				case opcode::BrGtEq:
					break;
				case opcode::BrLtEq:
					break;
				case opcode::Add:
					break;
				case opcode::Sub:
					break;
				case opcode::UMul:
					break;
				case opcode::UDiv:
					break;
				case opcode::SMul:
					break;
				case opcode::SDiv:
					break;
				case opcode::Shl:
					break;
				case opcode::Shr:
					break;
				case opcode::Neg:
					break;
				case opcode::And:
					break;
				case opcode::Or:
					break;
				case opcode::Xor:
					break;
				case opcode::Int:
					break;
				case opcode::Exec:
					break;
				}
			}
		private:
			pages iText;
			pages iStacks;
		};
	};
}
