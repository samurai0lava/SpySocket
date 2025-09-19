// Upload page functionality
document.addEventListener('DOMContentLoaded', function() {
    const fileInput = document.getElementById('file-input');
    const fileList = document.getElementById('file-list');
    const uploadBtn = document.getElementById('upload-btn');
    const clearBtn = document.getElementById('clear-btn');
    const uploadStatus = document.getElementById('upload-status');
    
    let selectedFile = null;

    // File input change handler
    fileInput.addEventListener('change', function(e) {
        handleFile(e.target.files[0]);
    });

    // Handle file selection (single file only)
    function handleFile(file) {
        selectedFile = file;
        updateFileList();
        updateUploadButton();
    }

    // Update file list display (single file)
    function updateFileList() {
        fileList.innerHTML = '';
        
        if (selectedFile) {
            const fileItem = document.createElement('div');
            fileItem.className = 'file-item';
            
            const fileName = document.createElement('span');
            fileName.className = 'file-name';
            fileName.textContent = selectedFile.name;
            
            const fileSize = document.createElement('span');
            fileSize.className = 'file-size';
            fileSize.textContent = formatFileSize(selectedFile.size);
            
            const removeBtn = document.createElement('button');
            removeBtn.className = 'remove-file';
            removeBtn.textContent = '×';
            removeBtn.onclick = () => removeFile();
            
            fileItem.appendChild(fileName);
            fileItem.appendChild(fileSize);
            fileItem.appendChild(removeBtn);
            fileList.appendChild(fileItem);
        }
    }

    // Remove file from selection
    function removeFile() {
        selectedFile = null;
        updateFileList();
        updateUploadButton();
    }

    // Update upload button state
    function updateUploadButton() {
        uploadBtn.disabled = selectedFile === null;
    }

    // Clear file
    clearBtn.addEventListener('click', function() {
        selectedFile = null;
        fileInput.value = '';
        updateFileList();
        updateUploadButton();
        hideStatus();
    });

    // Upload file
    uploadBtn.addEventListener('click', function(e) {
        e.preventDefault();
        
        if (!selectedFile) {
            return;
        }

        const formData = new FormData();
        formData.append('file', selectedFile);

        showStatus('loading', 'Uploading file...');
        uploadBtn.disabled = true;

        // Send POST request
        fetch('/upload', {
            method: 'POST',
            body: formData
        })
        .then(response => {
            if (response.ok) {
                return response.text();
            } else {
                throw new Error(`Upload failed with status: ${response.status}`);
            }
        })
        .then(data => {
            showStatus('success', `File uploaded successfully! Server response: ${data}`);
            // Clear file after successful upload
            selectedFile = null;
            fileInput.value = '';
            updateFileList();
        })
        .catch(error => {
            console.error('Upload error:', error);
            showStatus('error', `Upload failed: ${error.message}`);
        })
        .finally(() => {
            uploadBtn.disabled = false;
            updateUploadButton();
        });
    });

    // Show status message
    function showStatus(type, message) {
        uploadStatus.className = `upload-status ${type}`;
        uploadStatus.textContent = message;
        uploadStatus.style.display = 'block';
    }

    // Hide status message
    function hideStatus() {
        uploadStatus.style.display = 'none';
    }

    // Format file size
    function formatFileSize(bytes) {
        if (bytes === 0) return '0 Bytes';
        
        const k = 1024;
        const sizes = ['Bytes', 'KB', 'MB', 'GB'];
        const i = Math.floor(Math.log(bytes) / Math.log(k));
        
        return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
    }

    // Initialize
    updateUploadButton();
});