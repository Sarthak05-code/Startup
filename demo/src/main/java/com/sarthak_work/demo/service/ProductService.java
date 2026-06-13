package com.sarthak_work.demo.service;

import java.util.List;

import org.springframework.stereotype.Service;

import com.sarthak_work.demo.model.Product;
import com.sarthak_work.demo.repository.ProductRepository;

import lombok.RequiredArgsConstructor;

@Service
@RequiredArgsConstructor
public class ProductService {

    private final ProductRepository productRepository;

    public void deleteProduct(Long id) {
        productRepository.deleteById(id);
    }

    public Product getProductbyId(Long id) {
        return productRepository.findById(id)
                .orElseThrow(() -> new RuntimeException("This product was not Found."));
    }

    public List<Product> getProducts() {
        return productRepository.findAll();
    }

    public Product updateProduct(Long id, Product product) {
        Product existingProduct = productRepository.findById(id)
                .orElseThrow(() -> new RuntimeException("This product was not Found."));

        existingProduct.setName(product.getName());
        existingProduct.setDescription(product.getDescription());
        existingProduct.setCategory(product.getCategory());
        existingProduct.setPrice(product.getPrice());
        existingProduct.setStockQuantity(product.getStockQuantity());

        return productRepository.save(existingProduct);
    }

    public Product createProduct(Product product) {
        return productRepository.save(product);
    }

}
