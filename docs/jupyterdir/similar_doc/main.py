import io
import os
import uuid
import torch
import time
import random
import shutil

from transformers import AutoTokenizer, AutoModelForCausalLM

class MY_similar_doc:

    #---------------------------------------------------------------------- 
    # 생성자
    #---------------------------------------------------------------------- 
    def __init__(self, model_folder_path:str, device:str="cuda:0"):
        
        assert model_folder_path, f'model_folder_Path is empty'
        assert device, f'device is empty'
        
        self.model_path = model_folder_path
        self.device = device
        
        # 모델 로딩
        self.load_model()
        return
    #---------------------------------------------------------------------- 
    # 모델 로딩 : gemma-ko-2 로딩
    #---------------------------------------------------------------------- 
    def load_model(self):
        start_time = time.time()

        model = AutoModelForCausalLM.from_pretrained(self.model_path, device_map=self.device)
        tokenizer = AutoTokenizer.from_pretrained(self.model_path)
        
        self.model = model
        self.tokenizer = tokenizer
        
        # 소요된 시간을 계산합니다.
        end_time = time.time()
        formatted_elapsed_time = "{:.2f}".format(end_time - start_time)
        print(f'*[load_gemma] time:{formatted_elapsed_time}')

        return self.model, self.tokenizer
    #---------------------------------------------------------------------- 
    # 모델 저장.
    #---------------------------------------------------------------------- 
    def save_model(self, out_folder:str):
        self.model.save_pretrained(out_folder, safe_serialization=False)
        self.tokenizer.save_pretrained(out_folder)
    #---------------------------------------------------------------------- 
    # <bos> 테그 제거
    #---------------------------------------------------------------------- 
    def remove_bos_tag(self, input_string):
        # <bos> 태그를 제거합니다.
        if input_string.startswith("<bos>"):
            return input_string[len("<bos>"):].strip()
        return input_string
    #---------------------------------------------------------------------- 
    # 생성 : generate
    #---------------------------------------------------------------------- 
    def generate_text(self, input_text, max_length=70):
        if len(input_text) > 40:
            input_text = input_text[:40]

        input_ids = self.tokenizer(input_text, return_tensors="pt").to(self.device)
        outputs = self.model.generate(**input_ids, max_length=max_length)

        text = self.remove_bos_tag(self.tokenizer.decode(outputs[0]))
        return text
    
    #---------------------------------------------------------------------- 
    # 새로운 파일을 생성하는 함수 
    # => 원본파일에서 줄단위로 나누어서 %remove_per만큼 줄을 제거하고, %shuffle_per만큼 줄을 섞어서 원본과 유사한 파일을 생성하는 함수 
    # -in: srcFilePath : 원본 파일
    # -in: tgtFilePath : 새롭게 만들 파일
    # -in: remove_per : 원본파일에서 제거할 줄 % 
    # -in: shuffle_per : 원본파일에서 섞을 줄 %
    # -in: replace_per: 원본파일에서 대체할 줄 %
    #---------------------------------------------------------------------- 
    def create_similar_doc(self, srcFilePath:str, tgtFilePath:str, remove_per:int=10, shuffle_per:int=10, replace_per:int=10):

        assert srcFilePath, f"srcFilePath is Empty!!"
        assert tgtFilePath, f"tgtFilePath is Empty!!"
        #assert remove_per > 1, f"remove_per must be greater than 1!!"
        #assert shuffle_per > 1, f"shuffle_per must be greater than 1 1!!"

        msg:str = "success"
        #print(f'*shuffle_per:{shuffle_per}\n*remove_per:{remove_per}')

        try:
            # src 파일을 읽어온다.
            with open(srcFilePath, 'r', encoding='utf-8') as file:
                lines = file.readlines()
        except Exception as e:
            msg = f'readlines is Fail!!..(error: {e})'
            return 1001, msg

        # 전체 줄 수의 10% 계산합니다.
        total_lines = len(lines)
        remove_count:int = 0
        shuffle_count:int = 0
        replace_count:int = 0
        
        if remove_per > 0:
            remove_count = int(total_lines * (remove_per/100)) # 제거할 줄의 수
            
        if shuffle_per > 0:    
            shuffle_count = int(total_lines * (shuffle_per/100))     # 섞을 줄의 수
            
        if replace_per > 0:
            replace_count = int(total_lines * (replace_per/100)) # 치환할 줄의 수

        #print(f'*total_lines:{total_lines}\n*remove_count:{remove_count}\n*shuffle_count:{shuffle_count}\n*replace_count:{replace_count}\n')

        remove_indices = random.sample(range(total_lines), remove_count)
        lines = [line for i, line in enumerate(lines) if i not in remove_indices]

        # 남은 줄들 중에서 섞을 줄을 선택합니다.
        shuffle_indices = random.sample(range(len(lines)), shuffle_count)

        # 선택된 줄을 섞습니다.
        shuffled_lines = [lines[i] for i in shuffle_indices]
        random.shuffle(shuffled_lines)

        # 섞인 줄을 원래 위치에 다시 넣습니다.
        for i, index in enumerate(shuffle_indices):
            lines[index] = shuffled_lines[i]

        # 남은 줄들 중에서 치환할 줄을 선택합니다.
        replace_indices = random.sample(range(len(lines)), replace_count)

        # 선택된 줄을 치환합니다.
        for idx, index in enumerate(replace_indices):
            org_text = lines[index].strip() # 앞.뒤 공백 제거
            if len(org_text) > 5: # 5글자 이상인 경우에만 처리 
                new_text = self.generate_text(org_text)
                lines[index] = new_text
                
                #print(f'[{idx}]\n*원본: {org_text}\n*신규: {new_text}\n\n')
            
        # 새로운 디렉토리가 존재하지 않으면 생성합니다.
        tgt_dir = os.path.dirname(tgtFilePath)
        if not os.path.exists(tgt_dir):
            os.makedirs(tgt_dir)

        try:
            # 섞은 내용을 새로운 파일로 저장합니다.
            with open(tgtFilePath, 'w', encoding='utf-8') as file:
                file.writelines(lines)
        except Exception as e:
            msg = f'writelines is Fail!!..(error: {e})'
            return 1002, msg

        return 0, msg

