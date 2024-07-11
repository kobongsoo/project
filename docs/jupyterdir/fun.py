#---------------------------------------------------------------
# ==문서업로드 처리==
async def upload_doc01(file: UploadFile = File(...), file_folder:str='../../data11/docs'):
    assert file_folder, f"file_folder is empty!!"

    start_time = time.time()
    
    # ==이미지 타입인지 확인================
    mime_type = check_mime_type(file.content_type)    
    myutils.log_message(f'[info][/upload_doc01] *mime_type:{file.content_type}')
    
    # 원본파일을 저장할 org 폴더 지정
    org_folder = f"{file_folder}/org"
    if not os.path.exists(org_folder):
        os.makedirs(org_folder)
    
    # 추출 및 생성한 text 저장할 폴더 지정
    extract_folder = f"{file_folder}/extract"
    tgtPath = f"{extract_folder}/{file.filename}"
    if not os.path.exists(extract_folder):
        os.makedirs(extract_folder)
        
    # =====원본 파일 저장===================
    if mime_type == "txt":  # mime_type이 txt 면 추출하지 않고 extrac 폴더에 그냥 저장
        file_location = f"{extract_folder}/{file.filename}"
    else:
        file_location = f"{org_folder}/{file.filename}"
        
    with open(file_location, "wb") as f:
        content = await file.read()
        f.write(content)
    # ====================================
 
    # =====파일에서 text 추출/저장=========
    # 파일이 있고 mime_type=='doc'인 경우에만
    if os.path.isfile(file_location) and mime_type == "doc":
        myutils.log_message(f'[info][/upload01] *tgtPath:{tgtPath}')
        shaai.extract(srcPath=file_location, tgtPath=tgtPath)
    # ====================================
    
    # ==이미지는 vision모델로 text 생성/저장==
    if mime_type == "img":
        res = myvision.generate(image_path=file_location)
        
        # text 생성이 너무 짧은경우 1번더 다른 prompt로 text 생성함.
        if len(res) < 20:
            res += myvision.generate(image_path=file_location, prompt="What does the image show?")
        
        # 파일로 저장
        with open(tgtPath, 'w', encoding='utf-8') as file:
            file.write(res)
        
        myutils.log_message(f'[info][/upload01] *myvision.generate:\n{res}')
    # ====================================
    
    end_time = time.time()
    elapsed_time = "{:.2f}".format(end_time - start_time)
    
    response = {"extra_file_path": f"{tgtPath}", "mime_type":f"{file.content_type}", "img_to_text":{res}, "time": f"{elapsed_time}"}
    return response