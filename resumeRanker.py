from pypdf import (PdfReader)  # brings a tool that knows how to open PDF files and read their pages
from pathlib import Path  # Works with file path
import re  # used for searching patterns like email and phone number.


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
        "Education": ["education", "academic background", "qualifications"],
        "Experience": ["experience", "work experience", "employment history"],
        "Projects": ["projects", "academic projects", "personal projects"],
        "Certifications": ["certifications", "certificates", "licenses"],
    }

    found_sections = ([])  # Creates an empty list.it stores the names of the sections found in the resume.
    text = resume_text.lower()  # converts all the resume text in lower case

    for section, keywords in sections.items():
        for keyword in keywords:
            if keyword in text:
                found_sections.append(section)
                break

    return found_sections


# Extract useful features from resume text
def extract_features(resume_text, found_sections):

    # Count total words in the resume
    words = resume_text.split()
    word_count = len(words)

    # Count the number of detected sections
    section_count = len(found_sections)

    # Check whether an email address exists
    email_pattern = r"[\w.-]+@[\w.-]+\.\w+"
    email_found = bool(re.search(email_pattern, resume_text))

    # Check whether a 10-digit phone number exists
    phone_pattern = r"\b\d{10}\b"
    phone_found = bool(re.search(phone_pattern, resume_text))

    # Categorize technical skills
    skill_categories = {
        "Programming Languages": ["Python", "C", "C++", "Java", "JavaScript"],
        "Web Technologies": ["HTML", "CSS", "JavaScript"],
        "Databases": ["SQL", "DBMS", "MongoDB"],
        "Core CS": ["Data Structures", "Algorithms", "Operating System"],
        "AI / ML": ["Machine Learning", "Deep Learning"],
    }

    # Store detected skills by category
    found_skills = []
    skills_by_category = {}

    text = resume_text.lower()

    # Check each category
    for category, skills in skill_categories.items():

        category_found = []

        # Check each skill in the category
        for skill in skills:

            pattern = r"(?<!\w)" + re.escape(skill.lower()) + r"(?!\w)"

            if re.search(pattern, text):
                category_found.append(skill)

                if skill not in found_skills:
                    found_skills.append(skill)

        # Save category only if skills were found
        if category_found:
            skills_by_category[category] = category_found
    # # Extract education details
    # # Detect degree
    # degree_patterns = [
    #     r"\bB\.?\s?Tech\b",
    #     r"\bB\.?\s?E\.?\b",
    #     r"\bB\.?\s?Sc\b",
    #     r"\bBCA\b",
    #     r"\bM\.?\s?Tech\b",
    #     r"\bMCA\b",
    #     r"\bM\.?\s?Sc\b",
    #     r"\bMBA\b",
    # ]

    # degree_found = []

    # for pattern in degree_patterns:
    #     match = re.search(pattern, resume_text, re.IGNORECASE)

    #     if match:
    #         degree_found.append(match.group().strip())

    # # Detect institution name
    # institution_pattern = r"(?im)^.*(?:university|college|institute).*$"

    # institution_matches = re.findall(institution_pattern, resume_text)

    # institution_found = []

    # for institution in institution_matches:
    #     institution_found.append(institution.strip())

    # # Detect graduation year
    # year_pattern = r"\b20[2-9][0-9]\b"

    # graduation_years = re.findall(year_pattern, resume_text)
    # Store all features in a dictionary
    features = {
        "word_count": word_count,
        "section_count": section_count,
        "email_found": email_found,
        "phone_found": phone_found,
        "skills_found": found_skills,
        "skill_count": len(found_skills),
        "skills_by_category": skills_by_category,
        # "degree_found": degree_found,
        # "institution_found": institution_found,
        # "graduation_years": graduation_years,
    }

    return features


# Read and analyze the resume PDF
def analyze_resume():
    file_path = input(
        "\nEnter the PDF file path: "
    ).strip()  # strip()->removes the space from beginning and end.
    file_path = file_path.strip('"')  # strip()->removes the quotation from beginning and end.

    # check weather the file exists ro not.
    if not Path(file_path).is_file():  # path(file_path)->converts string into object.
        print("Error: File not found.")
        return

    try:
        reader = PdfReader(file_path)  # reads the pdf.(reader.pages) or creates a reader object
        resume_text = ""  # Empty string.for storing the pdf text in this

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
        # Extract resume features
        features = extract_features(resume_text, found_sections)

        print("\nResume Features:")
        print("Word Count:", features["word_count"])
        print("Section Count:", features["section_count"])
        print("Email Found:", features["email_found"])
        print("Phone Found:", features["phone_found"])
        print("Skill Count:", features["skill_count"])
        print("Skills Found:", features["skills_found"])
        print("\nSkills by Category:")
        for category, skills in features["skills_by_category"].items():
            print(category, ":", ", ".join(skills))
        # print("\nEducation Details:")
        # print("Degree:", features["degree_found"])
        # print("Institution:", features["institution_found"])
        # print("Graduation Year:", features["graduation_years"])
        # Display extracted text
        # print("\nExtracted resume text:")
        # print("-" * 40)
        # print(resume_text)
        # print("-" * 40)

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
