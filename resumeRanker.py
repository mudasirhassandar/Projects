
from pypdf import PdfReader  #brings a tool that knows how to open PDF files and read their pages
from pathlib import Path  #


# Display the main menu
def show_menu():
    print("-" * 40)
    print("       RESUMELENS")
    print("   ATS RESUME ANALYZER")
    print("-" * 40)

    print("\n1. Analyze Resume")
    print("2. View Previous Reports")
    print("3. Exit")


# Detect important resume sections
def detect_sections(resume_text):
    sections = {
        "Skills": ["skills", "technical skills", "key skills"],
        "Education": [
            "education",
            "academic background",
            "qualifications"
        ],
        "Experience": [
            "experience",
            "work experience",
            "employment history"
        ],
        "Projects": [
            "projects",
            "academic projects",
            "personal projects"
        ],
        "Certifications": [
            "certifications",
            "certificates",
            "licenses"
        ]
    }

    found_sections = []
    text = resume_text.lower()

    for section, keywords in sections.items():
        for keyword in keywords:
            if keyword in text:
                found_sections.append(section)
                break

    return found_sections


# Read and analyze the resume PDF
def analyze_resume():
    file_path = input("\nEnter the PDF file path: ").strip()
    file_path = file_path.strip('"')

    if not Path(file_path).is_file():
        print("Error: File not found.")
        return

    try:
        reader = PdfReader(file_path)
        resume_text = ""

        for page in reader.pages:
            resume_text += page.extract_text() or "\n"

        if not resume_text.strip():
            print("No readable text found in this PDF.")
            return

        print("\nResume text extracted successfully!")

        # Detect resume sections
        found_sections = detect_sections(resume_text)

        print("\nSections detected:")

        if found_sections:
            for section in found_sections:
                print("-", section)
        else:
            print("No recognized sections found.")

        # Display extracted text
        print("\nExtracted resume text:")
        print("-" * 40)
        print(resume_text)
        print("-" * 40)

    except Exception as error:
        print(f"Could not read PDF: {error}")


# Main program
while True:
    show_menu()
    choice = input("\nEnter your choice: ")

    match choice:
        case "1":
            analyze_resume()

        case "2":
            print("\nPrevious reports will be added later.")

        case "3":
            print("\nExiting ResumeLens. Goodbye!")
            break

        case _:
            print("\nInvalid choice. Please try again.")

    if choice != "3":
        input("\nPress Enter to return to the menu...")
        print()
