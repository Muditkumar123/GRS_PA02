# Helper Script to Generate PDF Report for PA02
# Usage: python3 MT25073_Report_Generator.py

from fpdf import FPDF
import os

class PDF(FPDF):
    def header(self):
        self.set_font('Arial', 'B', 14)
        self.cell(0, 10, 'PA02: Analysis of Network I/O Primitives', 0, 1, 'C')
        self.set_font('Arial', 'I', 10)
        self.cell(0, 5, 'Roll No: MT25073 | Course: CSE638 (Graduate Systems)', 0, 1, 'C')
        self.ln(10)

    def footer(self):
        self.set_y(-15)
        self.set_font('Arial', 'I', 8)
        self.cell(0, 10, f'Page {self.page_no()}', 0, 0, 'C')

    def chapter_title(self, title):
        self.set_font('Arial', 'B', 12)
        self.set_fill_color(220, 220, 220)
        self.cell(0, 8, title, 0, 1, 'L', 1)
        self.ln(4)

    def chapter_body(self, body):
        self.set_font('Arial', '', 11)
        # Ensure we are at the left margin to avoid "Not enough space" errors
        self.set_x(10) 
        self.multi_cell(0, 6, body)
        self.ln()

    def add_plot(self, image_path, width=160, caption=""):
        if os.path.exists(image_path):
            # Check if we need a page break before adding the image
            if self.get_y() + (width * 0.75) > 270: 
                self.add_page()
            
            # Center the image
            x = (210 - width) / 2
            self.image(image_path, x=x, w=width)
            self.ln(2)
            if caption:
                self.set_font('Arial', 'I', 9)
                self.cell(0, 5, caption, 0, 1, 'C')
            self.ln(5)
        else:
            self.set_text_color(255, 0, 0)
            self.cell(0, 10, f"Error: Image '{image_path}' not found.", 0, 1, 'C')
            self.set_text_color(0, 0, 0)

# Create PDF
pdf = PDF()
pdf.add_page()
pdf.set_auto_page_break(auto=True, margin=15)

# --------------------------------------------------------------------------
# SECTION 1: EXECUTION SCREENSHOTS
# --------------------------------------------------------------------------
pdf.chapter_title('1. Execution Screenshots')
pdf.chapter_body("Below are the screenshots of the Server and Client running concurrently.")

pdf.cell(0, 8, 'Server Output (Handling Requests):', 0, 1)
pdf.add_plot('screenshot_server.png', 180)

pdf.ln(5)
pdf.cell(0, 8, 'Client Output (Throughput Results):', 0, 1)
pdf.add_plot('screenshot_client.png', 180)

pdf.add_page()

# --------------------------------------------------------------------------
# SECTION 2: PERFORMANCE PLOTS
# --------------------------------------------------------------------------
pdf.chapter_title('2. Performance Plots')

# 2.1 Throughput
pdf.set_font('Arial', 'B', 11)
pdf.cell(0, 8, '2.1 Throughput vs Message Size', 0, 1)
pdf.add_plot('MT25073_Throughput_Grid.png', 170, "Fig 1: Throughput across varying message sizes.")

# 2.2 Latency
pdf.set_font('Arial', 'B', 11)
pdf.cell(0, 8, '2.2 Latency vs Thread Count', 0, 1)
pdf.add_plot('MT25073_Latency_Grid.png', 170, "Fig 2: Latency trends as thread count increases.")

pdf.add_page() # Force break to keep things clean

# 2.3 Cache Misses
pdf.set_font('Arial', 'B', 11)
pdf.cell(0, 8, '2.3 L1 Cache Misses vs Message Size', 0, 1)
pdf.add_plot('MT25073_Cache_Misses.png', 140, "Fig 3: L1 Cache Misses comparison (Threads=1).")

# 2.4 Efficiency
pdf.set_font('Arial', 'B', 11)
pdf.cell(0, 8, '2.4 CPU Cycles per Byte', 0, 1)
pdf.add_plot('MT25073_Efficiency_Grid.png', 170, "Fig 4: CPU efficiency comparison.")

pdf.add_page()

# --------------------------------------------------------------------------
# SECTION 3: ANALYSIS & REASONING (The 6 Questions)
# --------------------------------------------------------------------------
pdf.chapter_title('3. Analysis & Reasoning')

def add_qa(question, answer):
    pdf.set_x(10) # Reset margin safety
    pdf.set_font('Arial', 'B', 11)
    pdf.multi_cell(0, 6, question)
    pdf.set_font('Arial', '', 11)
    pdf.multi_cell(0, 6, answer)
    pdf.ln(3)

add_qa("Q1. Why does zero-copy not always give the best throughput?",
    "Zero-Copy introduces a fixed overhead for every operation (pinning pages, DMA setup, Error Queue notifications). "
    "For small messages (e.g., 1KB), this overhead exceeds the cost of a simple `memcpy`. "
    "Zero-Copy only wins when the payload is large enough (e.g., > 128KB) to justify the setup cost.")

add_qa("Q2. Which cache level shows the most reduction in misses and why?",
    "The L1 Data Cache shows the most reduction. Two-Copy floods L1 with streaming data during `memcpy`, causing cache pollution. "
    "Zero-Copy uses DMA, bypassing the CPU and L1 cache entirely for the payload, preserving cache lines for application data.")

add_qa("Q3. How does thread count interact with cache contention?",
    "Higher thread counts lead to frequent context switches and contention for the shared Last Level Cache (LLC). "
    "This 'thrashing' behavior degrades performance as threads fight for cache lines, visible in the latency spikes at 4-8 threads.")

add_qa("Q4. At what message size does one-copy outperform two-copy?",
    "One-Copy consistently matches or slightly outperforms Two-Copy from 1KB onwards. "
    "The benefit is most visible around 32KB, where eliminating the user-space copy provides a small latency gain without the heavy setup cost of Zero-Copy.")

add_qa("Q5. At what message size does zero-copy outperform two-copy?",
    "Zero-Copy overtakes Two-Copy between 128KB and 1MB. "
    "At 1MB, Zero-Copy maintains ~81 Gbps while Two-Copy drops to ~60 Gbps due to CPU saturation.")

add_qa("Q6. Identify one unexpected result.",
    "Unexpected: Zero-Copy throughput is extremely low (~3 Gbps) for 1KB messages. "
    "Explanation: The kernel syscall overhead (mode switching, page locking) dominates the execution time for small payloads, proving kernel-bypass is harmful for small latency-sensitive data.")

pdf.add_page()

# --------------------------------------------------------------------------
# SECTION 4: AI DECLARATION
# --------------------------------------------------------------------------
pdf.chapter_title('4. AI Usage Declaration')
pdf.chapter_body(
    "I used the Generative AI tool to assist with this assignment.\n"
    "Components Generated:\n"
    "1. Boilerplate Code: Initial socket connection setup for Client.c.\n"
    "2. Makefile: Generated the build script for multiple targets.\n"
    "3. Plotting: Generated the matplotlib logic for subplots.\n"
    "4. Debugging: Fixed 'SO_REUSEPORT' compilation errors.\n\n"
    "Core logic for Zero-Copy and One-Copy was implemented by me."
)

# --------------------------------------------------------------------------
# SECTION 5: GITHUB LINK
# --------------------------------------------------------------------------
pdf.chapter_title('5. GitHub Repository')
pdf.set_font('Arial', 'U', 11)
pdf.set_text_color(0, 0, 255)
pdf.cell(0, 10, 'https://github.com/Muditkumar123/GRS_PA02/tree/main', 0, 1, 'L', link='https://github.com/Muditkumar123/GRS_PA02/tree/main')
pdf.set_text_color(0, 0, 0)

# SAVE
pdf.output('MT25073_Report.pdf')
print("Success! MT25073_Report.pdf has been generated.")