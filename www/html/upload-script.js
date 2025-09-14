// Upload page functionality
document.addEventListener('DOMContentLoaded', function() {
    const fileInput = document.getElementById('file-input');
    const fileList = document.getElementById('file-list');
    const uploadBtn = document.getElementById('upload-btn');
    const clearBtn = document.getElementById('clear-btn');
    const uploadStatus = document.getElementById('upload-status');
    
    let selectedFiles = [];

    // File input change handler
    fileInput.addEventListener('change', function(e) {
        handleFiles(e.target.files);
    });

    // Handle file selection
    function handleFiles(files) {
        for (let i = 0; i < files.length; i++) {
            const file = files[i];
            if (!selectedFiles.find(f => f.name === file.name && f.size === file.size)) {
                selectedFiles.push(file);
            }
        }
        updateFileList();
        updateUploadButton();
    }

    // Update file list display
    function updateFileList() {
        fileList.innerHTML = '';
        
        selectedFiles.forEach((file, index) => {
            const fileItem = document.createElement('div');
            fileItem.className = 'file-item';
            
            const fileName = document.createElement('span');
            fileName.className = 'file-name';
            fileName.textContent = file.name;
            
            const fileSize = document.createElement('span');
            fileSize.className = 'file-size';
            fileSize.textContent = formatFileSize(file.size);
            
            const removeBtn = document.createElement('button');
            removeBtn.className = 'remove-file';
            removeBtn.textContent = '×';
            removeBtn.onclick = () => removeFile(index);
            
            fileItem.appendChild(fileName);
            fileItem.appendChild(fileSize);
            fileItem.appendChild(removeBtn);
            fileList.appendChild(fileItem);
        });
    }

    // Remove file from selection
    function removeFile(index) {
        selectedFiles.splice(index, 1);
        updateFileList();
        updateUploadButton();
    }

    // Update upload button state
    function updateUploadButton() {
        uploadBtn.disabled = selectedFiles.length === 0;
    }

    // Clear all files
    clearBtn.addEventListener('click', function() {
        selectedFiles = [];
        fileInput.value = '';
        updateFileList();
        updateUploadButton();
        hideStatus();
    });

    // Upload files
    uploadBtn.addEventListener('click', function(e) {
        e.preventDefault();
        
        if (selectedFiles.length === 0) {
            return;
        }

        const formData = new FormData();
        selectedFiles.forEach(file => {
            formData.append('files', file);
        });

        showStatus('loading', 'Uploading files...');
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
            showStatus('success', `Files uploaded successfully! Server response: ${data}`);
            // Clear files after successful upload
            selectedFiles = [];
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